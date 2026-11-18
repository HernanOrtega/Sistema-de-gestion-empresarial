#include "pedidosproveedores.h"
#include "ui_pedidosproveedores.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QDoubleValidator>

Pedidosproveedores::Pedidosproveedores(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Pedidosproveedores),
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
    ui->labelFechaPedido->setStyleSheet(labelStyle);
    ui->labelFechaEntregaEstimada->setStyleSheet(labelStyle);
    ui->labelEstado->setStyleSheet(labelStyle);
    ui->labelTotal->setStyleSheet(labelStyle);
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

    // Estilo para QDateTimeEdit y QDateEdit
    QString dateStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ff6666; border-radius: 4px; padding: 4px;";
    ui->dtFechaPedido->setStyleSheet(dateStyle);
    ui->deFechaEntregaEstimada->setStyleSheet(dateStyle);

    // Estilo para QTextEdit
    QString textEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ff6666; border-radius: 4px; padding: 4px;";
    ui->txtObservaciones->setStyleSheet(textEditStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaPedido->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaPedido->setDateTime(QDateTime::currentDateTime());

    ui->deFechaEntregaEstimada->setDisplayFormat("yyyy-MM-dd");
    ui->deFechaEntregaEstimada->setDate(QDate::currentDate().addDays(7)); // 7 días por defecto

    // Configurar combo de estados
    ui->cbEstado->clear();
    ui->cbEstado->addItem("PENDIENTE", "PENDIENTE");
    ui->cbEstado->addItem("CONFIRMADO", "CONFIRMADO");
    ui->cbEstado->addItem("EN_CAMINO", "EN_CAMINO");
    ui->cbEstado->addItem("ENTREGADO", "ENTREGADO");
    ui->cbEstado->addItem("CANCELADO", "CANCELADO");

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

    ui->tablaPedidosProveedores->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaPedidosProveedores->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Pedidosproveedores::filaSeleccionada);
}

Pedidosproveedores::~Pedidosproveedores()
{
    delete ui;
}

void Pedidosproveedores::cargarCombos()
{
    // Cargar proveedores activos - CORREGIDO
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

void Pedidosproveedores::cargarDatos()
{
    // CORREGIDO: Usar razon_social en lugar de nombre
    QString sql = R"(
        SELECT pp.id,
               p.razon_social AS proveedor,
               TO_CHAR(pp.fecha_pedido, 'YYYY-MM-DD HH24:MI') AS fecha_pedido,
               TO_CHAR(pp.fecha_entrega_estimada, 'YYYY-MM-DD') AS fecha_entrega_estimada,
               pp.total,
               pp.estado,
               pp.observaciones,
               u.username AS usuario
        FROM pedido_a_proveedor pp
        LEFT JOIN proveedor p ON pp.proveedor_id = p.id
        LEFT JOIN usuario u ON pp.usuario_id = u.id
        ORDER BY pp.fecha_pedido DESC
    )";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Proveedor"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Fecha Pedido"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Fecha Entrega Estimada"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Total"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Estado"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Observaciones"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Usuario"));

    ui->tablaPedidosProveedores->setModel(modelo);
    ui->tablaPedidosProveedores->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaPedidosProveedores->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaPedidosProveedores->horizontalHeader()->setStretchLastSection(true);
    ui->tablaPedidosProveedores->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Pedidosproveedores::limpiarCampos()
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

    ui->dtFechaPedido->setDateTime(QDateTime::currentDateTime());
    ui->deFechaEntregaEstimada->setDate(QDate::currentDate().addDays(7));

    ui->cbProveedor->setFocus();
}

void Pedidosproveedores::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Pedidosproveedores::on_btnGuardar_clicked()
{
    int proveedorId = ui->cbProveedor->currentData().toInt();
    QDateTime fechaPedido = ui->dtFechaPedido->dateTime();
    QDate fechaEntregaEstimada = ui->deFechaEntregaEstimada->date();
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

    if(fechaEntregaEstimada < QDate::currentDate()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha de entrega estimada no puede ser anterior a la fecha actual."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO pedido_a_proveedor (proveedor_id, fecha_pedido, fecha_entrega_estimada, total, estado, observaciones, usuario_id) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(proveedorId);
    query.addBindValue(fechaPedido);
    query.addBindValue(fechaEntregaEstimada);
    query.addBindValue(total);
    query.addBindValue(estado);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el pedido a proveedor:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Pedido a proveedor guardado correctamente."));
}

void Pedidosproveedores::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int proveedorId = ui->cbProveedor->currentData().toInt();
    QDateTime fechaPedido = ui->dtFechaPedido->dateTime();
    QDate fechaEntregaEstimada = ui->deFechaEntregaEstimada->date();
    double total = ui->txtTotal->text().toDouble();
    QString estado = ui->cbEstado->currentData().toString();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pedido para modificar."));
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

    if(fechaEntregaEstimada < QDate::currentDate()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha de entrega estimada no puede ser anterior a la fecha actual."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE pedido_a_proveedor SET proveedor_id=?, fecha_pedido=?, fecha_entrega_estimada=?, total=?, estado=?, observaciones=?, usuario_id=? WHERE id=?");
    query.addBindValue(proveedorId);
    query.addBindValue(fechaPedido);
    query.addBindValue(fechaEntregaEstimada);
    query.addBindValue(total);
    query.addBindValue(estado);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el pedido a proveedor:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Pedido a proveedor modificado correctamente."));
}

void Pedidosproveedores::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pedido para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este pedido a proveedor?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM pedido_a_proveedor WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el pedido a proveedor:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Pedido a proveedor eliminado correctamente."));
}

void Pedidosproveedores::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los IDs directamente de la base de datos para una búsqueda precisa
    int idPedido = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT proveedor_id, fecha_pedido, fecha_entrega_estimada, total, estado, observaciones, usuario_id "
                  "FROM pedido_a_proveedor WHERE id = ?");
    query.addBindValue(idPedido);

    if(query.exec() && query.next()) {
        int proveedorId = query.value(0).toInt();
        QDateTime fechaPedido = query.value(1).toDateTime();
        QDate fechaEntregaEstimada = query.value(2).toDate();
        double total = query.value(3).toDouble();
        QString estado = query.value(4).toString();
        QString observaciones = query.value(5).toString();
        int usuarioId = query.value(6).toInt();

        // Buscar por ID en lugar de por texto
        int indexProveedor = ui->cbProveedor->findData(proveedorId);
        if(indexProveedor >= 0) {
            ui->cbProveedor->setCurrentIndex(indexProveedor);
        }

        if(fechaPedido.isValid()) {
            ui->dtFechaPedido->setDateTime(fechaPedido);
        }

        if(fechaEntregaEstimada.isValid()) {
            ui->deFechaEntregaEstimada->setDate(fechaEntregaEstimada);
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
        qDebug() << "Error al cargar datos del pedido:" << query.lastError().text();
    }
}

void Pedidosproveedores::on_btnSalir_clicked()
{
    close();
}
