#include "ventas.h"
#include "ui_ventas.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Ventas::Ventas(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Ventas),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo rosa moderno ------------------------------
    this->setStyleSheet("background-color: #fff0f5;"); // Fondo rosa claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb6c1; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtTotal->setStyleSheet(lineEditStyle);
    ui->txtIVATotal->setStyleSheet(lineEditStyle);
    ui->txtNroFactura->setStyleSheet(lineEditStyle);
    ui->txtTimbrado->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #8b4789; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelCliente->setStyleSheet(labelStyle);
    ui->labelFechaVenta->setStyleSheet(labelStyle);
    ui->labelTotal->setStyleSheet(labelStyle);
    ui->labelIVATotal->setStyleSheet(labelStyle);
    ui->labelEstado->setStyleSheet(labelStyle);
    ui->labelMoneda->setStyleSheet(labelStyle);
    ui->labelNroFactura->setStyleSheet(labelStyle);
    ui->labelTimbrado->setStyleSheet(labelStyle);
    ui->labelUsuario->setStyleSheet(labelStyle);

    // Estilo para botones en tono rosa
    QString btnStyle = R"(
        QPushButton {
            background-color: #ff69b4;
            color: #ffffff;
            font-weight: bold;
            border-radius: 6px;
            padding: 6px 12px;
            border: none;
        }
        QPushButton:hover {
            background-color: #ff1493;
        }
        QPushButton:pressed {
            background-color: #db7093;
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
    QString comboStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb6c1; border-radius: 4px; padding: 4px;";
    ui->cbCliente->setStyleSheet(comboStyle);
    ui->cbEstado->setStyleSheet(comboStyle);
    ui->cbMoneda->setStyleSheet(comboStyle);
    ui->cbUsuario->setStyleSheet(comboStyle);

    // Estilo para QDateTimeEdit
    QString dateTimeStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb6c1; border-radius: 4px; padding: 4px;";
    ui->dtFechaVenta->setStyleSheet(dateTimeStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaVenta->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaVenta->setDateTime(QDateTime::currentDateTime());

    // Configurar combo de estados
    ui->cbEstado->clear();
    ui->cbEstado->addItem("PENDIENTE", "PENDIENTE");
    ui->cbEstado->addItem("CONFIRMADA", "CONFIRMADA");
    ui->cbEstado->addItem("ANULADA", "ANULADA");
    ui->cbEstado->addItem("FACTURADA", "FACTURADA");

    // Configurar validación para montos (solo números)
    ui->txtTotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));
    ui->txtIVATotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // ------------------------------ Configuración tabla en tonos rosas ------------------------------
    ui->tablaVentas->setStyleSheet(R"(
        QTableView {
            background-color: #fffafb;
            alternate-background-color: #ffeef1;
            gridline-color: #ffb6c1;
            selection-background-color: #ff69b4;
            selection-color: white;
            border: 1px solid #ffb6c1;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #db7093;
            color: white;
            font-weight: bold;
            border: 1px solid #c71585;
            padding: 6px;
        }
        QTableView::item {
            padding: 4px;
            border-bottom: 1px solid #ffd1dc;
            color: #333333;
        }
        QTableView::item:selected {
            background-color: #ff69b4;
            color: white;
        }
    )");

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaVentas->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Ventas::filaSeleccionada);
}

Ventas::~Ventas()
{
    delete ui;
}

void Ventas::cargarCombos()
{
    // Cargar clientes activos
    ui->cbCliente->clear();
    QSqlQuery query("SELECT id, nombre_apellido_cliente FROM cliente WHERE activo = true ORDER BY nombre_apellido_cliente");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbCliente->addItem(nombre, id);
    }

    // Cargar monedas
    ui->cbMoneda->clear();
    query.exec("SELECT id, nombre FROM moneda ORDER BY nombre");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbMoneda->addItem(nombre, id);
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

void Ventas::cargarDatos()
{
    QString sql = R"(
        SELECT v.id,
               c.nombre_apellido_cliente AS cliente,
               TO_CHAR(v.fecha_venta, 'YYYY-MM-DD HH24:MI') AS fecha_venta,
               v.total,
               v.iva_total,
               v.estado,
               m.nombre AS moneda,
               v.nro_factura,
               v.timbrado,
               u.username AS usuario
        FROM venta v
        LEFT JOIN cliente c ON v.cliente_id = c.id
        LEFT JOIN moneda m ON v.moneda_id = m.id
        LEFT JOIN usuario u ON v.usuario_id = u.id
        ORDER BY v.fecha_venta DESC
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Cliente"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Fecha Venta"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Total"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("IVA Total"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Estado"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Moneda"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Nro Factura"));
    modelo->setHeaderData(8, Qt::Horizontal, tr("Timbrado"));
    modelo->setHeaderData(9, Qt::Horizontal, tr("Usuario"));

    ui->tablaVentas->setModel(modelo);
    ui->tablaVentas->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaVentas->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaVentas->horizontalHeader()->setStretchLastSection(true);
    ui->tablaVentas->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Ventas::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtTotal->clear();
    ui->txtIVATotal->clear();
    ui->txtNroFactura->clear();
    ui->txtTimbrado->clear();

    if(ui->cbCliente->count() > 0)
        ui->cbCliente->setCurrentIndex(0);
    if(ui->cbEstado->count() > 0)
        ui->cbEstado->setCurrentIndex(0); // PENDIENTE por defecto
    if(ui->cbMoneda->count() > 0)
        ui->cbMoneda->setCurrentIndex(0);
    if(ui->cbUsuario->count() > 0)
        ui->cbUsuario->setCurrentIndex(0);

    ui->dtFechaVenta->setDateTime(QDateTime::currentDateTime());

    ui->cbCliente->setFocus();
}

void Ventas::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Ventas::on_btnGuardar_clicked()
{
    int clienteId = ui->cbCliente->currentData().toInt();
    QDateTime fechaVenta = ui->dtFechaVenta->dateTime();
    double total = ui->txtTotal->text().toDouble();
    double ivaTotal = ui->txtIVATotal->text().toDouble();
    QString estado = ui->cbEstado->currentData().toString();
    int monedaId = ui->cbMoneda->currentData().toInt();
    QString nroFactura = ui->txtNroFactura->text().trimmed();
    QString timbrado = ui->txtTimbrado->text().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(clienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cliente."));
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

    if(monedaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una moneda."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO venta (cliente_id, fecha_venta, total, iva_total, estado, moneda_id, nro_factura, timbrado, usuario_id) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(clienteId);
    query.addBindValue(fechaVenta);
    query.addBindValue(total);
    query.addBindValue(ivaTotal);
    query.addBindValue(estado);
    query.addBindValue(monedaId);
    query.addBindValue(nroFactura);
    query.addBindValue(timbrado);
    query.addBindValue(usuarioId);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar la venta:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Venta guardada correctamente."));
}

void Ventas::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int clienteId = ui->cbCliente->currentData().toInt();
    QDateTime fechaVenta = ui->dtFechaVenta->dateTime();
    double total = ui->txtTotal->text().toDouble();
    double ivaTotal = ui->txtIVATotal->text().toDouble();
    QString estado = ui->cbEstado->currentData().toString();
    int monedaId = ui->cbMoneda->currentData().toInt();
    QString nroFactura = ui->txtNroFactura->text().trimmed();
    QString timbrado = ui->txtTimbrado->text().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una venta para modificar."));
        return;
    }

    if(clienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cliente."));
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

    if(monedaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una moneda."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE venta SET cliente_id=?, fecha_venta=?, total=?, iva_total=?, estado=?, moneda_id=?, nro_factura=?, timbrado=?, usuario_id=? WHERE id=?");
    query.addBindValue(clienteId);
    query.addBindValue(fechaVenta);
    query.addBindValue(total);
    query.addBindValue(ivaTotal);
    query.addBindValue(estado);
    query.addBindValue(monedaId);
    query.addBindValue(nroFactura);
    query.addBindValue(timbrado);
    query.addBindValue(usuarioId);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar la venta:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Venta modificada correctamente."));
}

void Ventas::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una venta para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar esta venta?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM venta WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar la venta:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Venta eliminada correctamente."));
}

void Ventas::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    QString clienteNombre = modelo->data(modelo->index(row,1)).toString();
    int indexCliente = ui->cbCliente->findText(clienteNombre);
    if(indexCliente >= 0) ui->cbCliente->setCurrentIndex(indexCliente);

    QString fechaVentaStr = modelo->data(modelo->index(row,2)).toString();
    QDateTime fechaVenta = QDateTime::fromString(fechaVentaStr, "yyyy-MM-dd HH:mm");
    if(fechaVenta.isValid()) ui->dtFechaVenta->setDateTime(fechaVenta);

    ui->txtTotal->setText(modelo->data(modelo->index(row,3)).toString());
    ui->txtIVATotal->setText(modelo->data(modelo->index(row,4)).toString());

    QString estado = modelo->data(modelo->index(row,5)).toString();
    int indexEstado = ui->cbEstado->findData(estado);
    if(indexEstado >= 0) {
        ui->cbEstado->setCurrentIndex(indexEstado);
    } else {
        ui->cbEstado->setCurrentIndex(0); // PENDIENTE por defecto
    }

    QString monedaNombre = modelo->data(modelo->index(row,6)).toString();
    int indexMoneda = ui->cbMoneda->findText(monedaNombre);
    if(indexMoneda >= 0) ui->cbMoneda->setCurrentIndex(indexMoneda);

    ui->txtNroFactura->setText(modelo->data(modelo->index(row,7)).toString());
    ui->txtTimbrado->setText(modelo->data(modelo->index(row,8)).toString());

    QString usuarioNombre = modelo->data(modelo->index(row,9)).toString();
    int indexUsuario = ui->cbUsuario->findText(usuarioNombre);
    if(indexUsuario >= 0) ui->cbUsuario->setCurrentIndex(indexUsuario);
}

void Ventas::on_btnSalir_clicked()
{
    close();
}
