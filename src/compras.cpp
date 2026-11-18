#include "compras.h"
#include "ui_compras.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QDoubleValidator>

Compras::Compras(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Compras),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo rojo moderno ------------------------------
    this->setStyleSheet("background-color: #fff0f0;"); // Fondo rojo claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ff6666; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtTotal->setStyleSheet(lineEditStyle);
    ui->txtIVATotal->setStyleSheet(lineEditStyle);
    ui->txtNroFactura->setStyleSheet(lineEditStyle);
    ui->txtTimbrado->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #cc0000; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelProveedor->setStyleSheet(labelStyle);
    ui->labelFechaCompra->setStyleSheet(labelStyle);
    ui->labelTotal->setStyleSheet(labelStyle);
    ui->labelIVATotal->setStyleSheet(labelStyle);
    ui->labelEstado->setStyleSheet(labelStyle);
    ui->labelNroFactura->setStyleSheet(labelStyle);
    ui->labelTimbrado->setStyleSheet(labelStyle);
    ui->labelUsuario->setStyleSheet(labelStyle);

    // Estilo para botones en tono rojo
    QString btnStyle = R"(
        QPushButton {
            background-color: #ff4444;
            color: #ffffff;
            font-weight: bold;
            border-radius: 6px;
            padding: 6px 12px;
            border: none;
        }
        QPushButton:hover {
            background-color: #ff2222;
        }
        QPushButton:pressed {
            background-color: #cc0000;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
        }
    )";

    ui->btnNuevo->setStyleSheet(btnStyle);
    ui->btnGuardar->setStyleSheet(btnStyle);
    ui->btnModificar->setStyleSheet(btnStyle);
    ui->btnEliminar->setStyleSheet(btnStyle);
    ui->btnSalir->setStyleSheet(btnStyle);

    // Estilo para QComboBox
    QString comboStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ff6666; border-radius: 4px; padding: 4px;";
    ui->cbProveedor->setStyleSheet(comboStyle);
    ui->cbEstado->setStyleSheet(comboStyle);
    ui->cbUsuario->setStyleSheet(comboStyle);

    // Estilo para QDateTimeEdit
    QString dateTimeStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ff6666; border-radius: 4px; padding: 4px;";
    ui->dtFechaCompra->setStyleSheet(dateTimeStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaCompra->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaCompra->setDateTime(QDateTime::currentDateTime());

    // Configurar combo de estados
    ui->cbEstado->clear();
    ui->cbEstado->addItem("PENDIENTE", "PENDIENTE");
    ui->cbEstado->addItem("CONFIRMADA", "CONFIRMADA");
    ui->cbEstado->addItem("ANULADA", "ANULADA");
    ui->cbEstado->addItem("FACTURADA", "FACTURADA");

    // Configurar validación para montos (solo números)
    ui->txtTotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));
    ui->txtIVATotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // ------------------------------ Configuración tabla en tonos rojos ------------------------------
    QString tablaStyle = R"(
        QTableView {
            background-color: #fffafa;
            alternate-background-color: #ffe6e6;
            gridline-color: #ff6666;
            selection-background-color: #ff4444;
            selection-color: white;
            border: 1px solid #ff6666;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #cc0000;
            color: white;
            font-weight: bold;
            border: 1px solid #990000;
            padding: 6px;
        }
        QTableView::item {
            padding: 4px;
            border-bottom: 1px solid #ffb3b3;
            color: #333333;
        }
        QTableView::item:selected {
            background-color: #ff4444;
            color: white;
        }
    )";

    ui->tablaCompras->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaCompras->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Compras::filaSeleccionada);
}

Compras::~Compras()
{
    delete ui;
}

void Compras::cargarCombos()
{
    // Cargar proveedores activos
    ui->cbProveedor->clear();
    QSqlQuery query("SELECT id, razon_social FROM proveedor WHERE activo = true ORDER BY razon_social");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString razonSocial = query.value(1).toString();
        ui->cbProveedor->addItem(razonSocial, id);
    }

    // Cargar usuarios activos
    ui->cbUsuario->clear();
    query.exec("SELECT id, username FROM usuario WHERE activo = true ORDER BY username");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString username = query.value(1).toString();
        ui->cbUsuario->addItem(username, id);
    }
}

void Compras::cargarDatos()
{
    QString sql = R"(
        SELECT c.id,
               p.razon_social AS proveedor,
               TO_CHAR(c.fecha_compra, 'YYYY-MM-DD HH24:MI') AS fecha_compra,
               c.total,
               c.iva_total,
               c.estado,
               c.nro_factura,
               c.timbrado,
               u.username AS usuario
        FROM compra c
        LEFT JOIN proveedor p ON c.proveedor_id = p.id
        LEFT JOIN usuario u ON c.usuario_id = u.id
        ORDER BY c.fecha_compra DESC
    )";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Proveedor"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Fecha Compra"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Total"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("IVA Total"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Estado"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Nro Factura"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Timbrado"));
    modelo->setHeaderData(8, Qt::Horizontal, tr("Usuario"));

    ui->tablaCompras->setModel(modelo);
    ui->tablaCompras->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaCompras->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaCompras->horizontalHeader()->setStretchLastSection(true);
    ui->tablaCompras->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Compras::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtTotal->clear();
    ui->txtIVATotal->clear();
    ui->txtNroFactura->clear();
    ui->txtTimbrado->clear();

    if(ui->cbProveedor->count() > 0)
        ui->cbProveedor->setCurrentIndex(0);
    if(ui->cbEstado->count() > 0)
        ui->cbEstado->setCurrentIndex(0); // PENDIENTE por defecto
    if(ui->cbUsuario->count() > 0)
        ui->cbUsuario->setCurrentIndex(0);

    ui->dtFechaCompra->setDateTime(QDateTime::currentDateTime());

    ui->cbProveedor->setFocus();
}

void Compras::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Compras::on_btnGuardar_clicked()
{
    int proveedorId = ui->cbProveedor->currentData().toInt();
    QDateTime fechaCompra = ui->dtFechaCompra->dateTime();
    double total = ui->txtTotal->text().toDouble();
    double ivaTotal = ui->txtIVATotal->text().toDouble();
    QString estado = ui->cbEstado->currentData().toString();
    QString nroFactura = ui->txtNroFactura->text().trimmed();
    QString timbrado = ui->txtTimbrado->text().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(proveedorId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un proveedor."));
        return;
    }

    if(total <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El total debe ser mayor a cero."));
        return;
    }

    if(ivaTotal < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El IVA total no puede ser negativo."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO compra (proveedor_id, fecha_compra, total, iva_total, estado, nro_factura, timbrado, usuario_id) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(proveedorId);
    query.addBindValue(fechaCompra);
    query.addBindValue(total);
    query.addBindValue(ivaTotal);
    query.addBindValue(estado);
    query.addBindValue(nroFactura);
    query.addBindValue(timbrado);
    query.addBindValue(usuarioId);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar la compra:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Compra guardada correctamente."));
}

void Compras::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int proveedorId = ui->cbProveedor->currentData().toInt();
    QDateTime fechaCompra = ui->dtFechaCompra->dateTime();
    double total = ui->txtTotal->text().toDouble();
    double ivaTotal = ui->txtIVATotal->text().toDouble();
    QString estado = ui->cbEstado->currentData().toString();
    QString nroFactura = ui->txtNroFactura->text().trimmed();
    QString timbrado = ui->txtTimbrado->text().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una compra para modificar."));
        return;
    }

    if(proveedorId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un proveedor."));
        return;
    }

    if(total <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El total debe ser mayor a cero."));
        return;
    }

    if(ivaTotal < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El IVA total no puede ser negativo."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE compra SET proveedor_id=?, fecha_compra=?, total=?, iva_total=?, estado=?, nro_factura=?, timbrado=?, usuario_id=? WHERE id=?");
    query.addBindValue(proveedorId);
    query.addBindValue(fechaCompra);
    query.addBindValue(total);
    query.addBindValue(ivaTotal);
    query.addBindValue(estado);
    query.addBindValue(nroFactura);
    query.addBindValue(timbrado);
    query.addBindValue(usuarioId);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar la compra:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Compra modificada correctamente."));
}

void Compras::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una compra para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar esta compra?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM compra WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar la compra:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Compra eliminada correctamente."));
}

void Compras::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los IDs directamente de la base de datos para una búsqueda precisa
    int idCompra = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT proveedor_id, fecha_compra, total, iva_total, estado, nro_factura, timbrado, usuario_id "
                  "FROM compra WHERE id = ?");
    query.addBindValue(idCompra);

    if(query.exec() && query.next()) {
        int proveedorId = query.value(0).toInt();
        QDateTime fechaCompra = query.value(1).toDateTime();
        double total = query.value(2).toDouble();
        double ivaTotal = query.value(3).toDouble();
        QString estado = query.value(4).toString();
        QString nroFactura = query.value(5).toString();
        QString timbrado = query.value(6).toString();
        int usuarioId = query.value(7).toInt();

        // Buscar por ID en lugar de por texto
        int indexProveedor = ui->cbProveedor->findData(proveedorId);
        if(indexProveedor >= 0) {
            ui->cbProveedor->setCurrentIndex(indexProveedor);
        }

        if(fechaCompra.isValid()) {
            ui->dtFechaCompra->setDateTime(fechaCompra);
        }

        ui->txtTotal->setText(QString::number(total, 'f', 2));
        ui->txtIVATotal->setText(QString::number(ivaTotal, 'f', 2));

        int indexEstado = ui->cbEstado->findData(estado);
        if(indexEstado >= 0) {
            ui->cbEstado->setCurrentIndex(indexEstado);
        }

        ui->txtNroFactura->setText(nroFactura);
        ui->txtTimbrado->setText(timbrado);

        int indexUsuario = ui->cbUsuario->findData(usuarioId);
        if(indexUsuario >= 0) {
            ui->cbUsuario->setCurrentIndex(indexUsuario);
        }
    } else {
        qDebug() << "Error al cargar datos de la compra:" << query.lastError().text();
    }
}

void Compras::on_btnSalir_clicked()
{
    close();
}
