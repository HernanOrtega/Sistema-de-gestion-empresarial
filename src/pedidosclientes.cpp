#include "pedidosclientes.h"
#include "ui_pedidosclientes.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Pedidosclientes::Pedidosclientes(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Pedidosclientes),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo rosa moderno ------------------------------
    this->setStyleSheet("background-color: #fff0f5;"); // Fondo rosa claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb6c1; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtTotal->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #8b4789; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelCliente->setStyleSheet(labelStyle);
    ui->labelFechaPedido->setStyleSheet(labelStyle);
    ui->labelFechaEntrega->setStyleSheet(labelStyle);
    ui->labelEstado->setStyleSheet(labelStyle);
    ui->labelTotal->setStyleSheet(labelStyle);
    ui->labelObservaciones->setStyleSheet(labelStyle);
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
    ui->cbUsuario->setStyleSheet(comboStyle);

    // Estilo para QPlainTextEdit
    ui->txtObservaciones->setStyleSheet("background-color: #ffffff; color: #000000; border: 1px solid #ffb6c1; border-radius: 4px; padding: 4px;");

    // Estilo para QDateTimeEdit y QDateEdit
    QString dateTimeStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb6c1; border-radius: 4px; padding: 4px;";
    ui->dtFechaPedido->setStyleSheet(dateTimeStyle);
    ui->dtFechaEntrega->setStyleSheet(dateTimeStyle);

    ui->txtID->setEnabled(false);

    // Configurar formatos de fecha
    ui->dtFechaPedido->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaEntrega->setDisplayFormat("yyyy-MM-dd");
    ui->dtFechaPedido->setDateTime(QDateTime::currentDateTime());
    ui->dtFechaEntrega->setDate(QDate::currentDate().addDays(7)); // Entrega estimada en 7 días

    // Configurar combo de estados
    ui->cbEstado->clear();
    ui->cbEstado->addItem("PENDIENTE", "PENDIENTE");
    ui->cbEstado->addItem("CONFIRMADO", "CONFIRMADO");
    ui->cbEstado->addItem("EN PREPARACIÓN", "EN_PREPARACION");
    ui->cbEstado->addItem("EN CAMINO", "EN_CAMINO");
    ui->cbEstado->addItem("ENTREGADO", "ENTREGADO");
    ui->cbEstado->addItem("CANCELADO", "CANCELADO");

    // Configurar validación para total (solo números)
    ui->txtTotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // ------------------------------ Configuración tabla en tonos rosas ------------------------------
    ui->tablaPedidos->setStyleSheet(R"(
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
    connect(ui->tablaPedidos->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Pedidosclientes::filaSeleccionada);
}

Pedidosclientes::~Pedidosclientes()
{
    delete ui;
}

void Pedidosclientes::cargarCombos()
{
    // Cargar clientes activos
    ui->cbCliente->clear();
    QSqlQuery query("SELECT id, nombre_apellido_cliente FROM cliente WHERE activo = true ORDER BY nombre_apellido_cliente");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbCliente->addItem(nombre, id);
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

void Pedidosclientes::cargarDatos()
{
    QString sql = R"(
        SELECT pc.id,
               c.nombre_apellido_cliente AS cliente,
               TO_CHAR(pc.fecha_pedido, 'YYYY-MM-DD HH24:MI') AS fecha_pedido,
               TO_CHAR(pc.fecha_entrega_estimada, 'YYYY-MM-DD') AS fecha_entrega,
               pc.estado,
               pc.total,
               pc.observaciones,
               u.username AS usuario
        FROM pedido_cliente pc
        LEFT JOIN cliente c ON pc.cliente_id = c.id
        LEFT JOIN usuario u ON pc.usuario_id = u.id
        ORDER BY pc.fecha_pedido DESC
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Cliente"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Fecha Pedido"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Fecha Entrega"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Estado"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Total"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Observaciones"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Usuario"));

    ui->tablaPedidos->setModel(modelo);
    ui->tablaPedidos->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaPedidos->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaPedidos->horizontalHeader()->setStretchLastSection(true);
    ui->tablaPedidos->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Pedidosclientes::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtTotal->clear();
    ui->txtObservaciones->clear();

    if(ui->cbCliente->count() > 0)
        ui->cbCliente->setCurrentIndex(0);
    if(ui->cbEstado->count() > 0)
        ui->cbEstado->setCurrentIndex(0); // PENDIENTE por defecto
    if(ui->cbUsuario->count() > 0)
        ui->cbUsuario->setCurrentIndex(0);

    ui->dtFechaPedido->setDateTime(QDateTime::currentDateTime());
    ui->dtFechaEntrega->setDate(QDate::currentDate().addDays(7));

    ui->cbCliente->setFocus();
}

void Pedidosclientes::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Pedidosclientes::on_btnGuardar_clicked()
{
    int clienteId = ui->cbCliente->currentData().toInt();
    QDateTime fechaPedido = ui->dtFechaPedido->dateTime();
    QDate fechaEntrega = ui->dtFechaEntrega->date();
    QString estado = ui->cbEstado->currentData().toString();
    double total = ui->txtTotal->text().toDouble();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(clienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cliente."));
        return;
    }

    if(fechaEntrega < QDate::currentDate()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha de entrega no puede ser anterior a la fecha actual."));
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
    query.prepare("INSERT INTO pedido_cliente (cliente_id, fecha_pedido, fecha_entrega_estimada, estado, total, observaciones, usuario_id) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(clienteId);
    query.addBindValue(fechaPedido);
    query.addBindValue(fechaEntrega);
    query.addBindValue(estado);
    query.addBindValue(total);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el pedido:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Pedido guardado correctamente."));
}

void Pedidosclientes::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int clienteId = ui->cbCliente->currentData().toInt();
    QDateTime fechaPedido = ui->dtFechaPedido->dateTime();
    QDate fechaEntrega = ui->dtFechaEntrega->date();
    QString estado = ui->cbEstado->currentData().toString();
    double total = ui->txtTotal->text().toDouble();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pedido para modificar."));
        return;
    }

    if(clienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cliente."));
        return;
    }

    if(fechaEntrega < QDate::currentDate()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha de entrega no puede ser anterior a la fecha actual."));
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
    query.prepare("UPDATE pedido_cliente SET cliente_id=?, fecha_pedido=?, fecha_entrega_estimada=?, estado=?, total=?, observaciones=?, usuario_id=? WHERE id=?");
    query.addBindValue(clienteId);
    query.addBindValue(fechaPedido);
    query.addBindValue(fechaEntrega);
    query.addBindValue(estado);
    query.addBindValue(total);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el pedido:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Pedido modificado correctamente."));
}

void Pedidosclientes::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pedido para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este pedido?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM pedido_cliente WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el pedido:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Pedido eliminado correctamente."));
}

void Pedidosclientes::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
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

    QString fechaPedidoStr = modelo->data(modelo->index(row,2)).toString();
    QDateTime fechaPedido = QDateTime::fromString(fechaPedidoStr, "yyyy-MM-dd HH:mm");
    if(fechaPedido.isValid()) ui->dtFechaPedido->setDateTime(fechaPedido);

    QString fechaEntregaStr = modelo->data(modelo->index(row,3)).toString();
    QDate fechaEntrega = QDate::fromString(fechaEntregaStr, "yyyy-MM-dd");
    if(fechaEntrega.isValid()) {
        ui->dtFechaEntrega->setDate(fechaEntrega);
    } else {
        ui->dtFechaEntrega->setDate(QDate::currentDate().addDays(7));
    }

    // CORRECCIÓN: Buscar por el valor (data) en lugar del texto (display)
    QString estadoBD = modelo->data(modelo->index(row,4)).toString();
    int indexEstado = ui->cbEstado->findData(estadoBD);
    if(indexEstado >= 0) {
        ui->cbEstado->setCurrentIndex(indexEstado);
    } else {
        // Si no encuentra el estado, poner PENDIENTE por defecto
        ui->cbEstado->setCurrentIndex(0);
        qDebug() << "Estado no encontrado en combo:" << estadoBD;
    }

    ui->txtTotal->setText(modelo->data(modelo->index(row,5)).toString());
    ui->txtObservaciones->setPlainText(modelo->data(modelo->index(row,6)).toString());

    QString usuarioNombre = modelo->data(modelo->index(row,7)).toString();
    int indexUsuario = ui->cbUsuario->findText(usuarioNombre);
    if(indexUsuario >= 0) ui->cbUsuario->setCurrentIndex(indexUsuario);
}

void Pedidosclientes::on_btnSalir_clicked()
{
    close();
}
