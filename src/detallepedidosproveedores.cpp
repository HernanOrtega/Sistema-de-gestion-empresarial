#include "detallepedidosproveedores.h"
#include "ui_detallepedidosproveedores.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QIntValidator>
#include <QDoubleValidator>

Detallepedidosproveedores::Detallepedidosproveedores(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Detallepedidosproveedores),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo rojo moderno ------------------------------
    this->setStyleSheet("background-color: #fff0f0;"); // Fondo rojo claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ff6666; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtCantidad->setStyleSheet(lineEditStyle);
    ui->txtPrecioUnitario->setStyleSheet(lineEditStyle);
    ui->txtSubtotal->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #cc0000; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelPedidoProveedor->setStyleSheet(labelStyle);
    ui->labelProducto->setStyleSheet(labelStyle);
    ui->labelCantidad->setStyleSheet(labelStyle);
    ui->labelPrecioUnitario->setStyleSheet(labelStyle);
    ui->labelSubtotal->setStyleSheet(labelStyle);

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
    ui->cbPedidoProveedor->setStyleSheet(comboStyle);
    ui->cbProducto->setStyleSheet(comboStyle);

    ui->txtID->setEnabled(false);
    ui->txtSubtotal->setEnabled(false); // Se calcula automáticamente

    // Configurar validación para números
    ui->txtCantidad->setValidator(new QIntValidator(1, 999999, this));
    ui->txtPrecioUnitario->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // Conectar señales para cálculo automático del subtotal
    connect(ui->txtCantidad, &QLineEdit::textChanged, this, [this](const QString &) {
        calcularSubtotal();
    });
    connect(ui->txtPrecioUnitario, &QLineEdit::textChanged, this, [this](const QString &) {
        calcularSubtotal();
    });

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

    ui->tablaDetallePedidosProveedores->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaDetallePedidosProveedores->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Detallepedidosproveedores::filaSeleccionada);
}

Detallepedidosproveedores::~Detallepedidosproveedores()
{
    delete ui;
}

void Detallepedidosproveedores::calcularSubtotal()
{
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();

    double subtotal = cantidad * precioUnitario;
    ui->txtSubtotal->setText(QString::number(subtotal, 'f', 2));
}

void Detallepedidosproveedores::cargarCombos()
{
    // Cargar pedidos a proveedores activos
    ui->cbPedidoProveedor->clear();
    QSqlQuery query;

    QString pedidoQuery = "SELECT pp.id, 'Pedido #' || pp.id || ' - Proveedor: ' || COALESCE(p.razon_social, 'N/A') || ' - Total: ' || pp.total || ' - Estado: ' || pp.estado "
                          "FROM pedido_a_proveedor pp "
                          "LEFT JOIN proveedor p ON pp.proveedor_id = p.id "
                          "WHERE pp.estado != 'CANCELADO' "
                          "ORDER BY pp.fecha_pedido DESC";

    if(query.exec(pedidoQuery)) {
        while(query.next()) {
            int id = query.value(0).toInt();
            QString descripcion = query.value(1).toString();
            ui->cbPedidoProveedor->addItem(descripcion, id);
        }
    } else {
        qDebug() << "Error cargando pedidos a proveedores:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar los pedidos a proveedores:\n%1").arg(query.lastError().text()));
    }

    // Cargar productos activos
    ui->cbProducto->clear();
    QString productoQuery = "SELECT id, nombre FROM producto WHERE activo = true ORDER BY nombre";

    if(query.exec(productoQuery)) {
        while(query.next()) {
            int id = query.value(0).toInt();
            QString nombre = query.value(1).toString();
            ui->cbProducto->addItem(nombre, id);
        }
    } else {
        qDebug() << "Error cargando productos:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar los productos:\n%1").arg(query.lastError().text()));
    }
}

void Detallepedidosproveedores::cargarDatos()
{
    QString sql = "SELECT ppd.id, "
                  "'Pedido #' || pp.id || ' - Proveedor: ' || COALESCE(p.razon_social, 'N/A') AS pedido_proveedor, "
                  "pr.nombre AS producto, "
                  "ppd.cantidad, "
                  "ppd.precio_unitario, "
                  "ppd.subtotal "
                  "FROM pedido_a_proveedor_detalle ppd "
                  "LEFT JOIN pedido_a_proveedor pp ON ppd.pedido_a_proveedor_id = pp.id "
                  "LEFT JOIN proveedor p ON pp.proveedor_id = p.id "
                  "LEFT JOIN producto pr ON ppd.producto_id = pr.id "
                  "ORDER BY pp.fecha_pedido DESC, ppd.id";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Pedido Proveedor"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Producto"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Cantidad"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Precio Unitario"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Subtotal"));

    ui->tablaDetallePedidosProveedores->setModel(modelo);
    ui->tablaDetallePedidosProveedores->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaDetallePedidosProveedores->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaDetallePedidosProveedores->horizontalHeader()->setStretchLastSection(true);
    ui->tablaDetallePedidosProveedores->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Detallepedidosproveedores::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtCantidad->clear();
    ui->txtPrecioUnitario->clear();
    ui->txtSubtotal->clear();

    if(ui->cbPedidoProveedor->count() > 0)
        ui->cbPedidoProveedor->setCurrentIndex(0);
    if(ui->cbProducto->count() > 0)
        ui->cbProducto->setCurrentIndex(0);

    ui->cbPedidoProveedor->setFocus();
}

void Detallepedidosproveedores::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Detallepedidosproveedores::on_btnGuardar_clicked()
{
    int pedidoProveedorId = ui->cbPedidoProveedor->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(pedidoProveedorId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pedido a proveedor."));
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

    // Verificar si ya existe un detalle para este pedido a proveedor y producto
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM pedido_a_proveedor_detalle WHERE pedido_a_proveedor_id = ? AND producto_id = ?");
    checkQuery.addBindValue(pedidoProveedorId);
    checkQuery.addBindValue(productoId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe un detalle para este producto en el pedido a proveedor seleccionado."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO pedido_a_proveedor_detalle (pedido_a_proveedor_id, producto_id, cantidad, precio_unitario, subtotal) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(pedidoProveedorId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(subtotal);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el detalle del pedido a proveedor:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Detalle del pedido a proveedor guardado correctamente."));
}

void Detallepedidosproveedores::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int pedidoProveedorId = ui->cbPedidoProveedor->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle para modificar."));
        return;
    }

    if(pedidoProveedorId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pedido a proveedor."));
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

    // Verificar si ya existe otro detalle para este pedido a proveedor y producto (excluyendo el actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM pedido_a_proveedor_detalle WHERE pedido_a_proveedor_id = ? AND producto_id = ? AND id != ?");
    checkQuery.addBindValue(pedidoProveedorId);
    checkQuery.addBindValue(productoId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otro detalle para este producto en el pedido a proveedor seleccionado."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE pedido_a_proveedor_detalle SET pedido_a_proveedor_id=?, producto_id=?, cantidad=?, precio_unitario=?, subtotal=? WHERE id=?");
    query.addBindValue(pedidoProveedorId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(subtotal);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el detalle del pedido a proveedor:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Detalle del pedido a proveedor modificado correctamente."));
}

void Detallepedidosproveedores::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este detalle del pedido a proveedor?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM pedido_a_proveedor_detalle WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el detalle del pedido a proveedor:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Detalle del pedido a proveedor eliminado correctamente."));
}

void Detallepedidosproveedores::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los IDs directamente de la base de datos para una búsqueda precisa
    int idDetalle = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT pedido_a_proveedor_id, producto_id, cantidad, precio_unitario, subtotal "
                  "FROM pedido_a_proveedor_detalle WHERE id = ?");
    query.addBindValue(idDetalle);

    if(query.exec() && query.next()) {
        int pedidoProveedorId = query.value(0).toInt();
        int productoId = query.value(1).toInt();
        int cantidad = query.value(2).toInt();
        double precioUnitario = query.value(3).toDouble();
        double subtotal = query.value(4).toDouble();

        // Buscar por ID en lugar de por texto
        int indexPedidoProveedor = ui->cbPedidoProveedor->findData(pedidoProveedorId);
        if(indexPedidoProveedor >= 0) {
            ui->cbPedidoProveedor->setCurrentIndex(indexPedidoProveedor);
        }

        int indexProducto = ui->cbProducto->findData(productoId);
        if(indexProducto >= 0) {
            ui->cbProducto->setCurrentIndex(indexProducto);
        }

        ui->txtCantidad->setText(QString::number(cantidad));
        ui->txtPrecioUnitario->setText(QString::number(precioUnitario, 'f', 2));
        ui->txtSubtotal->setText(QString::number(subtotal, 'f', 2));
    } else {
        qDebug() << "Error al cargar datos del detalle:" << query.lastError().text();
    }
}

void Detallepedidosproveedores::on_btnSalir_clicked()
{
    close();
}
