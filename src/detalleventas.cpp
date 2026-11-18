#include "detalleventas.h"
#include "ui_detalleventas.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QIntValidator>
#include <QDoubleValidator>

Detalleventas::Detalleventas(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Detalleventas),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo rosa moderno ------------------------------
    this->setStyleSheet("background-color: #fff0f5;"); // Fondo rosa claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb6c1; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtCantidad->setStyleSheet(lineEditStyle);
    ui->txtPrecioUnitario->setStyleSheet(lineEditStyle);
    ui->txtIVA->setStyleSheet(lineEditStyle);
    ui->txtSubtotal->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #8b4789; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelVenta->setStyleSheet(labelStyle);
    ui->labelProducto->setStyleSheet(labelStyle);
    ui->labelCantidad->setStyleSheet(labelStyle);
    ui->labelPrecioUnitario->setStyleSheet(labelStyle);
    ui->labelIVA->setStyleSheet(labelStyle);
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
    ui->cbVenta->setStyleSheet(comboStyle);
    ui->cbProducto->setStyleSheet(comboStyle);

    ui->txtID->setEnabled(false);
    ui->txtSubtotal->setEnabled(false); // Se calcula automáticamente

    // Configurar validación para números
    ui->txtCantidad->setValidator(new QIntValidator(1, 999999, this));
    ui->txtPrecioUnitario->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));
    ui->txtIVA->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // Conectar señales para cálculo automático del subtotal - CORREGIDO
    connect(ui->txtCantidad, &QLineEdit::textChanged, this, [this](const QString &) {
        calcularSubtotal();
    });
    connect(ui->txtPrecioUnitario, &QLineEdit::textChanged, this, [this](const QString &) {
        calcularSubtotal();
    });
    connect(ui->txtIVA, &QLineEdit::textChanged, this, [this](const QString &) {
        calcularSubtotal();
    });

    // ------------------------------ Configuración tabla en tonos rosas ------------------------------
    ui->tablaDetalleVentas->setStyleSheet(R"(
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
    connect(ui->tablaDetalleVentas->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Detalleventas::filaSeleccionada);
}

Detalleventas::~Detalleventas()
{
    delete ui;
}

void Detalleventas::calcularSubtotal()
{
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double iva = ui->txtIVA->text().toDouble();

    double subtotal = (cantidad * precioUnitario) + iva;
    ui->txtSubtotal->setText(QString::number(subtotal, 'f', 2));
}

void Detalleventas::cargarCombos()
{
    // Cargar ventas activas
    ui->cbVenta->clear();
    QSqlQuery query("SELECT id, CONCAT('Venta #', id, ' - ', TO_CHAR(fecha_venta, 'YYYY-MM-DD')) FROM venta WHERE estado != 'ANULADA' ORDER BY fecha_venta DESC");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString descripcion = query.value(1).toString();
        ui->cbVenta->addItem(descripcion, id);
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

void Detalleventas::cargarDatos()
{
    QString sql = R"(
        SELECT vd.id,
               CONCAT('Venta #', v.id, ' - ', TO_CHAR(v.fecha_venta, 'YYYY-MM-DD')) AS venta,
               p.nombre AS producto,
               vd.cantidad,
               vd.precio_unitario,
               vd.iva,
               vd.subtotal
        FROM venta_detalle vd
        LEFT JOIN venta v ON vd.venta_id = v.id
        LEFT JOIN producto p ON vd.producto_id = p.id
        ORDER BY v.fecha_venta DESC, vd.id
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Venta"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Producto"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Cantidad"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Precio Unitario"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("IVA"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Subtotal"));

    ui->tablaDetalleVentas->setModel(modelo);
    ui->tablaDetalleVentas->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaDetalleVentas->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaDetalleVentas->horizontalHeader()->setStretchLastSection(true);
    ui->tablaDetalleVentas->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Detalleventas::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtCantidad->clear();
    ui->txtPrecioUnitario->clear();
    ui->txtIVA->clear();
    ui->txtSubtotal->clear();

    if(ui->cbVenta->count() > 0)
        ui->cbVenta->setCurrentIndex(0);
    if(ui->cbProducto->count() > 0)
        ui->cbProducto->setCurrentIndex(0);

    ui->cbVenta->setFocus();
}

void Detalleventas::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Detalleventas::on_btnGuardar_clicked()
{
    int ventaId = ui->cbVenta->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double iva = ui->txtIVA->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(ventaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una venta."));
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

    if(iva < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El IVA no puede ser negativo."));
        return;
    }

    // Verificar si ya existe un detalle para esta venta y producto
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM venta_detalle WHERE venta_id = ? AND producto_id = ?");
    checkQuery.addBindValue(ventaId);
    checkQuery.addBindValue(productoId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe un detalle para este producto en la venta seleccionada."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO venta_detalle (venta_id, producto_id, cantidad, precio_unitario, iva, subtotal) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(ventaId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(iva);
    query.addBindValue(subtotal);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el detalle de venta:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Detalle de venta guardado correctamente."));
}

void Detalleventas::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int ventaId = ui->cbVenta->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double iva = ui->txtIVA->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle para modificar."));
        return;
    }

    if(ventaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una venta."));
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

    if(iva < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El IVA no puede ser negativo."));
        return;
    }

    // Verificar si ya existe otro detalle para esta venta y producto (excluyendo el actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM venta_detalle WHERE venta_id = ? AND producto_id = ? AND id != ?");
    checkQuery.addBindValue(ventaId);
    checkQuery.addBindValue(productoId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otro detalle para este producto en la venta seleccionada."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE venta_detalle SET venta_id=?, producto_id=?, cantidad=?, precio_unitario=?, iva=?, subtotal=? WHERE id=?");
    query.addBindValue(ventaId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(iva);
    query.addBindValue(subtotal);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el detalle de venta:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Detalle de venta modificado correctamente."));
}

void Detalleventas::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este detalle de venta?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM venta_detalle WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el detalle de venta:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Detalle de venta eliminado correctamente."));
}

void Detalleventas::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    QString ventaDesc = modelo->data(modelo->index(row,1)).toString();
    int indexVenta = ui->cbVenta->findText(ventaDesc);
    if(indexVenta >= 0) ui->cbVenta->setCurrentIndex(indexVenta);

    QString productoNombre = modelo->data(modelo->index(row,2)).toString();
    int indexProducto = ui->cbProducto->findText(productoNombre);
    if(indexProducto >= 0) ui->cbProducto->setCurrentIndex(indexProducto);

    ui->txtCantidad->setText(modelo->data(modelo->index(row,3)).toString());
    ui->txtPrecioUnitario->setText(modelo->data(modelo->index(row,4)).toString());
    ui->txtIVA->setText(modelo->data(modelo->index(row,5)).toString());
    ui->txtSubtotal->setText(modelo->data(modelo->index(row,6)).toString());
}

void Detalleventas::on_btnSalir_clicked()
{
    close();
}
