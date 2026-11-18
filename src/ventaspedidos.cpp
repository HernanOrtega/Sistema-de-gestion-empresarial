#include "ventaspedidos.h"
#include "ui_ventaspedidos.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Ventaspedidos::Ventaspedidos(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Ventaspedidos),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo rosa moderno ------------------------------
    this->setStyleSheet("background-color: #fff0f5;"); // Fondo rosa claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb6c1; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #8b4789; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelPedidoCliente->setStyleSheet(labelStyle);
    ui->labelVenta->setStyleSheet(labelStyle);
    ui->labelFechaAsociacion->setStyleSheet(labelStyle);

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
    ui->cbPedidoCliente->setStyleSheet(comboStyle);
    ui->cbVenta->setStyleSheet(comboStyle);

    // Estilo para QDateTimeEdit
    QString dateTimeStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb6c1; border-radius: 4px; padding: 4px;";
    ui->dtFechaAsociacion->setStyleSheet(dateTimeStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaAsociacion->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaAsociacion->setDateTime(QDateTime::currentDateTime());

    // ------------------------------ Configuración tabla en tonos rosas ------------------------------
    ui->tablaVentasPedidos->setStyleSheet(R"(
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
    connect(ui->tablaVentasPedidos->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Ventaspedidos::filaSeleccionada);
}

Ventaspedidos::~Ventaspedidos()
{
    delete ui;
}

void Ventaspedidos::cargarCombos()
{
    // Cargar pedidos de cliente activos
    ui->cbPedidoCliente->clear();
    QSqlQuery query;

    QString pedidoQuery = "SELECT pc.id, 'Pedido #' || pc.id || ' - Cliente: ' || COALESCE(c.nombre_apellido_cliente, 'N/A') || ' - Total: ' || pc.total "
                          "FROM pedido_cliente pc "
                          "LEFT JOIN cliente c ON pc.cliente_id = c.id "
                          "WHERE pc.estado != 'ANULADO' ORDER BY pc.fecha_pedido DESC";

    if(query.exec(pedidoQuery)) {
        while(query.next()) {
            int id = query.value(0).toInt();
            QString descripcion = query.value(1).toString();
            ui->cbPedidoCliente->addItem(descripcion, id);
        }
    } else {
        qDebug() << "Error cargando pedidos:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar los pedidos:\n%1").arg(query.lastError().text()));
    }

    // Cargar ventas activas
    ui->cbVenta->clear();

    QString ventaQuery = "SELECT v.id, 'Venta #' || v.id || ' - ' || TO_CHAR(v.fecha_venta, 'YYYY-MM-DD') || ' - Cliente: ' || COALESCE(c.nombre_apellido_cliente, 'N/A') || ' - Total: ' || v.total "
                         "FROM venta v "
                         "LEFT JOIN cliente c ON v.cliente_id = c.id "
                         "WHERE v.estado != 'ANULADA' ORDER BY v.fecha_venta DESC";

    if(query.exec(ventaQuery)) {
        while(query.next()) {
            int id = query.value(0).toInt();
            QString descripcion = query.value(1).toString();
            ui->cbVenta->addItem(descripcion, id);
        }
    } else {
        qDebug() << "Error cargando ventas:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar las ventas:\n%1").arg(query.lastError().text()));
    }
}

void Ventaspedidos::cargarDatos()
{
    QString sql = "SELECT vp.id, "
                  "'Pedido #' || pc.id || ' - Cliente: ' || COALESCE(c1.nombre_apellido_cliente, 'N/A') AS pedido_cliente, "
                  "'Venta #' || v.id || ' - ' || TO_CHAR(v.fecha_venta, 'YYYY-MM-DD') || ' - Cliente: ' || COALESCE(c2.nombre_apellido_cliente, 'N/A') AS venta, "
                  "TO_CHAR(vp.fecha_asociacion, 'YYYY-MM-DD HH24:MI') AS fecha_asociacion "
                  "FROM pedido_cliente_has_venta vp "
                  "LEFT JOIN pedido_cliente pc ON vp.pedido_cliente_id = pc.id "
                  "LEFT JOIN venta v ON vp.venta_id = v.id "
                  "LEFT JOIN cliente c1 ON pc.cliente_id = c1.id "
                  "LEFT JOIN cliente c2 ON v.cliente_id = c2.id "
                  "ORDER BY vp.fecha_asociacion DESC";

    // CORREGIDO: setQuery es void, no retorna bool
    modelo->setQuery(sql);

    // Verificar si hubo error en la consulta
    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Pedido Cliente"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Venta"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Fecha Asociación"));

    ui->tablaVentasPedidos->setModel(modelo);
    ui->tablaVentasPedidos->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaVentasPedidos->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaVentasPedidos->horizontalHeader()->setStretchLastSection(true);
    ui->tablaVentasPedidos->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Ventaspedidos::limpiarCampos()
{
    ui->txtID->clear();

    if(ui->cbPedidoCliente->count() > 0)
        ui->cbPedidoCliente->setCurrentIndex(0);
    if(ui->cbVenta->count() > 0)
        ui->cbVenta->setCurrentIndex(0);

    ui->dtFechaAsociacion->setDateTime(QDateTime::currentDateTime());

    ui->cbPedidoCliente->setFocus();
}

void Ventaspedidos::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Ventaspedidos::on_btnGuardar_clicked()
{
    int pedidoClienteId = ui->cbPedidoCliente->currentData().toInt();
    int ventaId = ui->cbVenta->currentData().toInt();
    QDateTime fechaAsociacion = ui->dtFechaAsociacion->dateTime();

    if(pedidoClienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pedido de cliente."));
        return;
    }

    if(ventaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una venta."));
        return;
    }

    // Verificar si ya existe una asociación para este pedido y venta
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM pedido_cliente_has_venta WHERE pedido_cliente_id = ? AND venta_id = ?");
    checkQuery.addBindValue(pedidoClienteId);
    checkQuery.addBindValue(ventaId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe una asociación para este pedido y venta."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO pedido_cliente_has_venta (pedido_cliente_id, venta_id, fecha_asociacion) "
                  "VALUES (?, ?, ?)");
    query.addBindValue(pedidoClienteId);
    query.addBindValue(ventaId);
    query.addBindValue(fechaAsociacion);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar la asociación pedido-venta:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Asociación pedido-venta guardada correctamente."));
}

void Ventaspedidos::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int pedidoClienteId = ui->cbPedidoCliente->currentData().toInt();
    int ventaId = ui->cbVenta->currentData().toInt();
    QDateTime fechaAsociacion = ui->dtFechaAsociacion->dateTime();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una asociación para modificar."));
        return;
    }

    if(pedidoClienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pedido de cliente."));
        return;
    }

    if(ventaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una venta."));
        return;
    }

    // Verificar si ya existe otra asociación para este pedido y venta (excluyendo la actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM pedido_cliente_has_venta WHERE pedido_cliente_id = ? AND venta_id = ? AND id != ?");
    checkQuery.addBindValue(pedidoClienteId);
    checkQuery.addBindValue(ventaId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otra asociación para este pedido y venta."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE pedido_cliente_has_venta SET pedido_cliente_id=?, venta_id=?, fecha_asociacion=? WHERE id=?");
    query.addBindValue(pedidoClienteId);
    query.addBindValue(ventaId);
    query.addBindValue(fechaAsociacion);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar la asociación pedido-venta:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Asociación pedido-venta modificada correctamente."));
}

void Ventaspedidos::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una asociación para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar esta asociación pedido-venta?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM pedido_cliente_has_venta WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar la asociación pedido-venta:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Asociación pedido-venta eliminada correctamente."));
}

void Ventaspedidos::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los IDs directamente de la base de datos para una búsqueda precisa
    int idAsociacion = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT pedido_cliente_id, venta_id, fecha_asociacion "
                  "FROM pedido_cliente_has_venta WHERE id = ?");
    query.addBindValue(idAsociacion);

    if(query.exec() && query.next()) {
        int pedidoClienteId = query.value(0).toInt();
        int ventaId = query.value(1).toInt();
        QDateTime fechaAsociacion = query.value(2).toDateTime();

        // Buscar por ID en lugar de por texto
        int indexPedido = ui->cbPedidoCliente->findData(pedidoClienteId);
        if(indexPedido >= 0) {
            ui->cbPedidoCliente->setCurrentIndex(indexPedido);
        } else {
            qDebug() << "No se encontró el pedido con ID:" << pedidoClienteId;
        }

        int indexVenta = ui->cbVenta->findData(ventaId);
        if(indexVenta >= 0) {
            ui->cbVenta->setCurrentIndex(indexVenta);
        } else {
            qDebug() << "No se encontró la venta con ID:" << ventaId;
        }

        if(fechaAsociacion.isValid()) {
            ui->dtFechaAsociacion->setDateTime(fechaAsociacion);
        }
    } else {
        qDebug() << "Error al cargar datos de la asociación:" << query.lastError().text();
    }
}

void Ventaspedidos::on_btnSalir_clicked()
{
    close();
}
