#include "ordenescompra.h"
#include "ui_ordenescompra.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QDoubleValidator>

Ordenescompra::Ordenescompra(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Ordenescompra),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo rojo moderno ------------------------------
    this->setStyleSheet("background-color: #fff0f0;"); // Fondo rojo claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ff6666; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtTotal->setStyleSheet(lineEditStyle);
    ui->txtObservaciones->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #cc0000; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelProveedor->setStyleSheet(labelStyle);
    ui->labelFechaOrden->setStyleSheet(labelStyle);
    ui->labelTotal->setStyleSheet(labelStyle);
    ui->labelEstado->setStyleSheet(labelStyle);
    ui->labelObservaciones->setStyleSheet(labelStyle);
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
    QString dateStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ff6666; border-radius: 4px; padding: 4px;";
    ui->dtFechaOrden->setStyleSheet(dateStyle);

    // Estilo para QTextEdit
    QString textEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ff6666; border-radius: 4px; padding: 4px;";
    ui->txtObservaciones->setStyleSheet(textEditStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaOrden->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaOrden->setDateTime(QDateTime::currentDateTime());

    // Configurar combo de estados
    ui->cbEstado->clear();
    ui->cbEstado->addItem("PENDIENTE", "PENDIENTE");
    ui->cbEstado->addItem("APROBADA", "APROBADA");
    ui->cbEstado->addItem("RECHAZADA", "RECHAZADA");
    ui->cbEstado->addItem("EN_PROCESO", "EN_PROCESO");
    ui->cbEstado->addItem("COMPLETADA", "COMPLETADA");
    ui->cbEstado->addItem("CANCELADA", "CANCELADA");

    // Configurar validación para montos (solo números)
    ui->txtTotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

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

    ui->tablaOrdenesCompra->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaOrdenesCompra->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Ordenescompra::filaSeleccionada);
}

Ordenescompra::~Ordenescompra()
{
    delete ui;
}

void Ordenescompra::cargarCombos()
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

void Ordenescompra::cargarDatos()
{
    QString sql = R"(
        SELECT oc.id,
               p.razon_social AS proveedor,
               TO_CHAR(oc.fecha_orden, 'YYYY-MM-DD HH24:MI') AS fecha_orden,
               oc.total,
               oc.estado,
               oc.observaciones,
               u.username AS usuario
        FROM orden_de_compra oc
        LEFT JOIN proveedor p ON oc.proveedor_id = p.id
        LEFT JOIN usuario u ON oc.usuario_id = u.id
        ORDER BY oc.fecha_orden DESC
    )";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Proveedor"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Fecha Orden"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Total"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Estado"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Observaciones"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Usuario"));

    ui->tablaOrdenesCompra->setModel(modelo);
    ui->tablaOrdenesCompra->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaOrdenesCompra->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaOrdenesCompra->horizontalHeader()->setStretchLastSection(true);
    ui->tablaOrdenesCompra->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Ordenescompra::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtTotal->clear();
    ui->txtObservaciones->clear();

    if(ui->cbProveedor->count() > 0)
        ui->cbProveedor->setCurrentIndex(0);
    if(ui->cbEstado->count() > 0)
        ui->cbEstado->setCurrentIndex(0); // PENDIENTE por defecto
    if(ui->cbUsuario->count() > 0)
        ui->cbUsuario->setCurrentIndex(0);

    ui->dtFechaOrden->setDateTime(QDateTime::currentDateTime());

    ui->cbProveedor->setFocus();
}

void Ordenescompra::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Ordenescompra::on_btnGuardar_clicked()
{
    int proveedorId = ui->cbProveedor->currentData().toInt();
    QDateTime fechaOrden = ui->dtFechaOrden->dateTime();
    double total = ui->txtTotal->text().toDouble();
    QString estado = ui->cbEstado->currentData().toString();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(proveedorId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un proveedor."));
        return;
    }

    if(total <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El total debe ser mayor a cero."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO orden_de_compra (proveedor_id, fecha_orden, total, estado, observaciones, usuario_id) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(proveedorId);
    query.addBindValue(fechaOrden);
    query.addBindValue(total);
    query.addBindValue(estado);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar la orden de compra:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Orden de compra guardada correctamente."));
}

void Ordenescompra::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int proveedorId = ui->cbProveedor->currentData().toInt();
    QDateTime fechaOrden = ui->dtFechaOrden->dateTime();
    double total = ui->txtTotal->text().toDouble();
    QString estado = ui->cbEstado->currentData().toString();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una orden de compra para modificar."));
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

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE orden_de_compra SET proveedor_id=?, fecha_orden=?, total=?, estado=?, observaciones=?, usuario_id=? WHERE id=?");
    query.addBindValue(proveedorId);
    query.addBindValue(fechaOrden);
    query.addBindValue(total);
    query.addBindValue(estado);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar la orden de compra:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Orden de compra modificada correctamente."));
}

void Ordenescompra::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una orden de compra para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar esta orden de compra?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM orden_de_compra WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar la orden de compra:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Orden de compra eliminada correctamente."));
}

void Ordenescompra::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los IDs directamente de la base de datos para una búsqueda precisa
    int idOrden = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT proveedor_id, fecha_orden, total, estado, observaciones, usuario_id "
                  "FROM orden_de_compra WHERE id = ?");
    query.addBindValue(idOrden);

    if(query.exec() && query.next()) {
        int proveedorId = query.value(0).toInt();
        QDateTime fechaOrden = query.value(1).toDateTime();
        double total = query.value(2).toDouble();
        QString estado = query.value(3).toString();
        QString observaciones = query.value(4).toString();
        int usuarioId = query.value(5).toInt();

        // Buscar por ID en lugar de por texto
        int indexProveedor = ui->cbProveedor->findData(proveedorId);
        if(indexProveedor >= 0) {
            ui->cbProveedor->setCurrentIndex(indexProveedor);
        }

        if(fechaOrden.isValid()) {
            ui->dtFechaOrden->setDateTime(fechaOrden);
        }

        ui->txtTotal->setText(QString::number(total, 'f', 2));

        int indexEstado = ui->cbEstado->findData(estado);
        if(indexEstado >= 0) {
            ui->cbEstado->setCurrentIndex(indexEstado);
        }

        ui->txtObservaciones->setPlainText(observaciones);

        int indexUsuario = ui->cbUsuario->findData(usuarioId);
        if(indexUsuario >= 0) {
            ui->cbUsuario->setCurrentIndex(indexUsuario);
        }
    } else {
        qDebug() << "Error al cargar datos de la orden de compra:" << query.lastError().text();
    }
}

void Ordenescompra::on_btnSalir_clicked()
{
    close();
}
