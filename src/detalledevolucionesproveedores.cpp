#include "detalledevolucionesproveedores.h"
#include "ui_detalledevolucionesproveedores.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QIntValidator>
#include <QDoubleValidator>

Detalledevolucionesproveedores::Detalledevolucionesproveedores(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Detalledevolucionesproveedores),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo cian moderno ------------------------------
    this->setStyleSheet("background-color: #f0fdff;"); // Fondo cian claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtCantidad->setStyleSheet(lineEditStyle);
    ui->txtPrecioUnitario->setStyleSheet(lineEditStyle);
    ui->txtSubtotal->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #00838f; font-weight: bold; font-size: 14px;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelDevolucion->setStyleSheet(labelStyle);
    ui->labelProducto->setStyleSheet(labelStyle);
    ui->labelCantidad->setStyleSheet(labelStyle);
    ui->labelPrecioUnitario->setStyleSheet(labelStyle);
    ui->labelSubtotal->setStyleSheet(labelStyle);

    // Estilo para botones en tono cian moderno
    QString btnStyle = R"(
        QPushButton {
            background-color: #00bcd4;
            color: #ffffff;
            font-weight: bold;
            border-radius: 8px;
            padding: 8px 16px;
            border: none;
            font-size: 14px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #00acc1;
        }
        QPushButton:pressed {
            background-color: #0097a7;
            background-color: #00838f;
        }
        QPushButton:disabled {
            background-color: #b2ebf2;
            color: #80deea;
        }
    )";

    ui->btnNuevo->setStyleSheet(btnStyle);
    ui->btnGuardar->setStyleSheet(btnStyle);
    ui->btnModificar->setStyleSheet(btnStyle);
    ui->btnEliminar->setStyleSheet(btnStyle);
    ui->btnSalir->setStyleSheet(btnStyle);

    // Estilo para QComboBox
    QString comboStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->cbDevolucion->setStyleSheet(comboStyle);
    ui->cbProducto->setStyleSheet(comboStyle);

    ui->txtID->setEnabled(false);
    ui->txtSubtotal->setEnabled(false); // El subtotal se calcula automáticamente

    // Configurar validación para cantidades y precios
    ui->txtCantidad->setValidator(new QIntValidator(1, 999999, this));
    ui->txtPrecioUnitario->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // Conectar señales para calcular subtotal automáticamente
    connect(ui->txtCantidad, &QLineEdit::textChanged, this, &Detalledevolucionesproveedores::calcularSubtotal);
    connect(ui->txtPrecioUnitario, &QLineEdit::textChanged, this, &Detalledevolucionesproveedores::calcularSubtotal);

    // ------------------------------ Configuración tabla en tonos cian ------------------------------
    QString tablaStyle = R"(
        QTableView {
            background-color: #e0f7fa;
            alternate-background-color: #b2ebf2;
            gridline-color: #00bcd4;
            selection-background-color: #00bcd4;
            selection-color: white;
            border: 2px solid #00bcd4;
            border-radius: 8px;
            font-size: 13px;
        }
        QHeaderView::section {
            background-color: #00838f;
            color: white;
            font-weight: bold;
            border: 1px solid #006064;
            padding: 8px;
            font-size: 14px;
        }
        QTableView::item {
            padding: 6px;
            border-bottom: 1px solid #80deea;
            color: #006064;
        }
        QTableView::item:selected {
            background-color: #00bcd4;
            color: white;
            font-weight: bold;
        }
        QTableView::item:hover {
            background-color: #b2ebf2;
        }
    )";

    ui->tablaDetallesDevolucion->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    if (ui->tablaDetallesDevolucion->selectionModel()) {
        connect(ui->tablaDetallesDevolucion->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                &Detalledevolucionesproveedores::filaSeleccionada);
    }
}

Detalledevolucionesproveedores::~Detalledevolucionesproveedores()
{
    delete ui;
}

void Detalledevolucionesproveedores::cargarCombos()
{
    // Cargar devoluciones a proveedores
    ui->cbDevolucion->clear();
    QSqlQuery query;
    if (!query.exec("SELECT dp.id, 'Devolución #' || dp.id || ' - Prov: ' || p.razon_social || ' - Total: ' || dp.total || ' - ' || TO_CHAR(dp.fecha_devolucion, 'YYYY-MM-DD') "
                    "FROM devolucion_a_proveedor dp "
                    "LEFT JOIN proveedor p ON dp.proveedor_id = p.id "
                    "ORDER BY dp.fecha_devolucion DESC")) {
        qDebug() << "Error cargando devoluciones:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar las devoluciones:\n%1").arg(query.lastError().text()));
        return;
    }

    while(query.next()) {
        int id = query.value(0).toInt();
        QString descripcion = query.value(1).toString();
        ui->cbDevolucion->addItem(descripcion, id);
    }

    // Cargar productos activos
    ui->cbProducto->clear();
    if (!query.exec("SELECT id, nombre, codigo FROM producto WHERE activo = true ORDER BY nombre")) {
        qDebug() << "Error cargando productos:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar los productos:\n%1").arg(query.lastError().text()));
        return;
    }

    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        QString codigo = query.value(2).toString();
        QString displayText = QString("%1 (%2)").arg(nombre).arg(codigo);
        ui->cbProducto->addItem(displayText, id);
    }
}

void Detalledevolucionesproveedores::cargarDatos()
{
    QString sql = "SELECT dpd.id, "
                  "'Devolución #' || dp.id || ' - ' || p.razon_social AS devolucion, "
                  "pr.nombre || ' (' || pr.codigo || ')' AS producto, "
                  "dpd.cantidad, "
                  "dpd.precio_unitario, "
                  "dpd.subtotal "
                  "FROM devolucion_proveedor_detalle dpd "
                  "LEFT JOIN devolucion_a_proveedor dp ON dpd.devolucion_a_proveedor_id = dp.id "
                  "LEFT JOIN proveedor p ON dp.proveedor_id = p.id "
                  "LEFT JOIN producto pr ON dpd.producto_id = pr.id "
                  "ORDER BY dp.fecha_devolucion DESC, pr.nombre";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Devolución"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Producto"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Cantidad"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Precio Unitario"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Subtotal"));

    ui->tablaDetallesDevolucion->setModel(modelo);
    ui->tablaDetallesDevolucion->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaDetallesDevolucion->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaDetallesDevolucion->horizontalHeader()->setStretchLastSection(true);
    ui->tablaDetallesDevolucion->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Ajustar el ancho de las columnas
    ui->tablaDetallesDevolucion->setColumnWidth(0, 50);   // ID
    ui->tablaDetallesDevolucion->setColumnWidth(1, 200);  // Devolución
    ui->tablaDetallesDevolucion->setColumnWidth(2, 200);  // Producto
    ui->tablaDetallesDevolucion->setColumnWidth(3, 80);   // Cantidad
    ui->tablaDetallesDevolucion->setColumnWidth(4, 100);  // Precio Unitario
    ui->tablaDetallesDevolucion->setColumnWidth(5, 100);  // Subtotal

    limpiarCampos();
}

void Detalledevolucionesproveedores::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtCantidad->clear();
    ui->txtPrecioUnitario->clear();
    ui->txtSubtotal->clear();

    if(ui->cbDevolucion->count() > 0)
        ui->cbDevolucion->setCurrentIndex(0);
    if(ui->cbProducto->count() > 0)
        ui->cbProducto->setCurrentIndex(0);

    ui->cbDevolucion->setFocus();
}

void Detalledevolucionesproveedores::calcularSubtotal()
{
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double subtotal = cantidad * precioUnitario;

    ui->txtSubtotal->setText(QString::number(subtotal, 'f', 2));
}

void Detalledevolucionesproveedores::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Detalledevolucionesproveedores::on_btnGuardar_clicked()
{
    int devolucionId = ui->cbDevolucion->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(devolucionId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una devolución."));
        ui->cbDevolucion->setFocus();
        return;
    }

    if(productoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un producto."));
        ui->cbProducto->setFocus();
        return;
    }

    if(cantidad <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad debe ser mayor a cero."));
        ui->txtCantidad->setFocus();
        return;
    }

    if(precioUnitario <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El precio unitario debe ser mayor a cero."));
        ui->txtPrecioUnitario->setFocus();
        return;
    }

    // Verificar si ya existe un detalle para esta devolución y producto
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM devolucion_proveedor_detalle WHERE devolucion_a_proveedor_id = ? AND producto_id = ?");
    checkQuery.addBindValue(devolucionId);
    checkQuery.addBindValue(productoId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe un detalle para esta devolución y producto."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO devolucion_proveedor_detalle (devolucion_a_proveedor_id, producto_id, cantidad, precio_unitario, subtotal) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(devolucionId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(subtotal);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el detalle de devolución:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Detalle de devolución guardado correctamente."));
}

void Detalledevolucionesproveedores::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int devolucionId = ui->cbDevolucion->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle de devolución para modificar."));
        return;
    }

    if(devolucionId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una devolución."));
        ui->cbDevolucion->setFocus();
        return;
    }

    if(productoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un producto."));
        ui->cbProducto->setFocus();
        return;
    }

    if(cantidad <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad debe ser mayor a cero."));
        ui->txtCantidad->setFocus();
        return;
    }

    if(precioUnitario <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El precio unitario debe ser mayor a cero."));
        ui->txtPrecioUnitario->setFocus();
        return;
    }

    // Verificar si ya existe otro detalle para esta devolución y producto (excluyendo el actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM devolucion_proveedor_detalle WHERE devolucion_a_proveedor_id = ? AND producto_id = ? AND id != ?");
    checkQuery.addBindValue(devolucionId);
    checkQuery.addBindValue(productoId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otro detalle para esta devolución y producto."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE devolucion_proveedor_detalle SET devolucion_a_proveedor_id=?, producto_id=?, cantidad=?, precio_unitario=?, subtotal=? WHERE id=?");
    query.addBindValue(devolucionId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(subtotal);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el detalle de devolución:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Detalle de devolución modificado correctamente."));
}

void Detalledevolucionesproveedores::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle de devolución para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este detalle de devolución?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM devolucion_proveedor_detalle WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el detalle de devolución:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Detalle de devolución eliminado correctamente."));
}

void Detalledevolucionesproveedores::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los datos directamente de la base de datos para una búsqueda precisa
    int idDetalle = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT devolucion_a_proveedor_id, producto_id, cantidad, precio_unitario, subtotal "
                  "FROM devolucion_proveedor_detalle WHERE id = ?");
    query.addBindValue(idDetalle);

    if(query.exec() && query.next()) {
        int devolucionId = query.value(0).toInt();
        int productoId = query.value(1).toInt();
        int cantidad = query.value(2).toInt();
        double precioUnitario = query.value(3).toDouble();
        double subtotal = query.value(4).toDouble();

        // Buscar por ID en lugar de por texto
        int indexDevolucion = ui->cbDevolucion->findData(devolucionId);
        if(indexDevolucion >= 0) {
            ui->cbDevolucion->setCurrentIndex(indexDevolucion);
        }

        int indexProducto = ui->cbProducto->findData(productoId);
        if(indexProducto >= 0) {
            ui->cbProducto->setCurrentIndex(indexProducto);
        }

        ui->txtCantidad->setText(QString::number(cantidad));
        ui->txtPrecioUnitario->setText(QString::number(precioUnitario, 'f', 2));
        ui->txtSubtotal->setText(QString::number(subtotal, 'f', 2));
    } else {
        qDebug() << "Error al cargar datos del detalle de devolución:" << query.lastError().text();
    }
}

void Detalledevolucionesproveedores::on_btnSalir_clicked()
{
    close();
}
