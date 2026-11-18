#include "detallepedidosclientes.h"
#include "ui_detallepedidosclientes.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Detallepedidosclientes::Detallepedidosclientes(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Detallepedidosclientes),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo rosa moderno ------------------------------
    this->setStyleSheet("background-color: #fff0f5;"); // Fondo rosa claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb6c1; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtCantidad->setStyleSheet(lineEditStyle);
    ui->txtPrecioUnitario->setStyleSheet(lineEditStyle);
    ui->txtSubtotal->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #8b4789; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelPedido->setStyleSheet(labelStyle);
    ui->labelProducto->setStyleSheet(labelStyle);
    ui->labelCantidad->setStyleSheet(labelStyle);
    ui->labelPrecioUnitario->setStyleSheet(labelStyle);
    ui->labelSubtotal->setStyleSheet(labelStyle);

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
    ui->cbPedido->setStyleSheet(comboStyle);
    ui->cbProducto->setStyleSheet(comboStyle);

    ui->txtID->setEnabled(false);
    ui->txtSubtotal->setEnabled(false); // El subtotal se calcula automáticamente

    // Configurar validación para números
    ui->txtCantidad->setValidator(new QIntValidator(1, 999999, this));
    ui->txtPrecioUnitario->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));
    ui->txtSubtotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // Conectar señales para cálculo automático del subtotal
    connect(ui->txtCantidad, &QLineEdit::textChanged, this, &Detallepedidosclientes::calcularSubtotal);
    connect(ui->txtPrecioUnitario, &QLineEdit::textChanged, this, &Detallepedidosclientes::calcularSubtotal);

    // ------------------------------ Configuración tabla en tonos rosas ------------------------------
    ui->tablaDetalles->setStyleSheet(R"(
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
    connect(ui->tablaDetalles->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Detallepedidosclientes::filaSeleccionada);
}

Detallepedidosclientes::~Detallepedidosclientes()
{
    delete ui;
}

void Detallepedidosclientes::cargarCombos()
{
    // Cargar pedidos de clientes
    ui->cbPedido->clear();
    QSqlQuery query(R"(
        SELECT pc.id,
               'Pedido #' || pc.id || ' - ' || c.nombre_apellido_cliente || ' - ' || pc.total || ' - ' || pc.estado
        FROM pedido_cliente pc
        LEFT JOIN cliente c ON pc.cliente_id = c.id
        ORDER BY pc.id DESC
    )");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString descripcion = query.value(1).toString();
        ui->cbPedido->addItem(descripcion, id);
    }

    // Cargar productos activos
    ui->cbProducto->clear();
    query.exec("SELECT id, nombre FROM producto WHERE activo = true ORDER BY nombre");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbProducto->addItem(nombre, id);
    }
}

void Detallepedidosclientes::cargarDatos()
{
    QString sql = R"(
        SELECT pcd.id,
               'Pedido #' || pc.id || ' - ' || c.nombre_apellido_cliente AS pedido,
               p.nombre AS producto,
               pcd.cantidad,
               pcd.precio_unitario,
               pcd.subtotal
        FROM pedido_cliente_detalle pcd
        LEFT JOIN pedido_cliente pc ON pcd.pedido_cliente_id = pc.id
        LEFT JOIN cliente c ON pc.cliente_id = c.id
        LEFT JOIN producto p ON pcd.producto_id = p.id
        ORDER BY pcd.id DESC
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Pedido"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Producto"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Cantidad"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Precio Unitario"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Subtotal"));

    ui->tablaDetalles->setModel(modelo);
    ui->tablaDetalles->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaDetalles->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaDetalles->horizontalHeader()->setStretchLastSection(true);
    ui->tablaDetalles->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Detallepedidosclientes::calcularSubtotal()
{
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double subtotal = cantidad * precioUnitario;

    ui->txtSubtotal->setText(QString::number(subtotal, 'f', 2));
}

void Detallepedidosclientes::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtCantidad->clear();
    ui->txtPrecioUnitario->clear();
    ui->txtSubtotal->clear();

    if(ui->cbPedido->count() > 0)
        ui->cbPedido->setCurrentIndex(0);
    if(ui->cbProducto->count() > 0)
        ui->cbProducto->setCurrentIndex(0);

    ui->cbPedido->setFocus();
}

void Detallepedidosclientes::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Detallepedidosclientes::on_btnGuardar_clicked()
{
    int pedidoId = ui->cbPedido->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(pedidoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pedido."));
        return;
    }

    if(productoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un producto."));
        return;
    }

    if(cantidad <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad debe ser mayor a cero."));
        return;
    }

    if(precioUnitario <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El precio unitario debe ser mayor a cero."));
        return;
    }

    // Verificar que el subtotal calculado coincida
    double subtotalCalculado = cantidad * precioUnitario;
    if(qAbs(subtotal - subtotalCalculado) > 0.01) {
        QMessageBox::warning(this, tr("Atención"), tr("El subtotal no coincide con el cálculo. Recalcule el subtotal."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO pedido_cliente_detalle (pedido_cliente_id, producto_id, cantidad, precio_unitario, subtotal) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(pedidoId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(subtotal);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el detalle del pedido:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Detalle de pedido guardado correctamente."));
}

void Detallepedidosclientes::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int pedidoId = ui->cbPedido->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle de pedido para modificar."));
        return;
    }

    if(pedidoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pedido."));
        return;
    }

    if(productoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un producto."));
        return;
    }

    if(cantidad <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad debe ser mayor a cero."));
        return;
    }

    if(precioUnitario <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El precio unitario debe ser mayor a cero."));
        return;
    }

    // Verificar que el subtotal calculado coincida
    double subtotalCalculado = cantidad * precioUnitario;
    if(qAbs(subtotal - subtotalCalculado) > 0.01) {
        QMessageBox::warning(this, tr("Atención"), tr("El subtotal no coincide con el cálculo. Recalcule el subtotal."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE pedido_cliente_detalle SET pedido_cliente_id=?, producto_id=?, cantidad=?, precio_unitario=?, subtotal=? WHERE id=?");
    query.addBindValue(pedidoId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(subtotal);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el detalle del pedido:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Detalle de pedido modificado correctamente."));
}

void Detallepedidosclientes::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle de pedido para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este detalle de pedido?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM pedido_cliente_detalle WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el detalle del pedido:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Detalle de pedido eliminado correctamente."));
}

void Detallepedidosclientes::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // CORRECCIÓN: Buscar el pedido por ID en lugar de por texto
    QString pedidoDesc = modelo->data(modelo->index(row,1)).toString();

    // Extraer el ID del pedido de la descripción (asumiendo formato: "Pedido #ID - ...")
    int pedidoId = -1;
    QRegularExpression regex("Pedido #(\\d+)");
    QRegularExpressionMatch match = regex.match(pedidoDesc);
    if (match.hasMatch()) {
        pedidoId = match.captured(1).toInt();
    }

    // Buscar en el combo por el ID (data) en lugar del texto
    int indexPedido = -1;
    if (pedidoId != -1) {
        indexPedido = ui->cbPedido->findData(pedidoId);
    }

    // Si no encuentra por ID, intentar por texto (fallback)
    if (indexPedido == -1) {
        indexPedido = ui->cbPedido->findText(pedidoDesc);
    }

    if(indexPedido >= 0) {
        ui->cbPedido->setCurrentIndex(indexPedido);
    } else {
        qDebug() << "No se pudo encontrar el pedido:" << pedidoDesc;
    }

    QString productoNombre = modelo->data(modelo->index(row,2)).toString();
    int indexProducto = ui->cbProducto->findText(productoNombre);
    if(indexProducto >= 0) ui->cbProducto->setCurrentIndex(indexProducto);

    ui->txtCantidad->setText(modelo->data(modelo->index(row,3)).toString());
    ui->txtPrecioUnitario->setText(modelo->data(modelo->index(row,4)).toString());
    ui->txtSubtotal->setText(modelo->data(modelo->index(row,5)).toString());
}

void Detallepedidosclientes::on_txtCantidad_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    calcularSubtotal();
}

void Detallepedidosclientes::on_txtPrecioUnitario_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    calcularSubtotal();
}

void Detallepedidosclientes::on_btnSalir_clicked()
{
    close();
}
