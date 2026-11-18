#include "productos.h"
#include "ui_productos.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Productos::Productos(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Productos),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo moderno ------------------------------
    this->setStyleSheet("background-color: #f0f0f5;");

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border:1px solid #ccc; border-radius:4px; padding:4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtCodigo->setStyleSheet(lineEditStyle);
    ui->txtNombre->setStyleSheet(lineEditStyle);
    ui->txtPrecioVenta->setStyleSheet(lineEditStyle);
    ui->txtPrecioCompra->setStyleSheet(lineEditStyle);
    ui->txtStockMinimo->setStyleSheet(lineEditStyle);
    ui->txtIVA->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #333333; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelCodigo->setStyleSheet(labelStyle);
    ui->labelNombre->setStyleSheet(labelStyle);
    ui->labelDescripcion->setStyleSheet(labelStyle);
    ui->labelPrecioVenta->setStyleSheet(labelStyle);
    ui->labelPrecioCompra->setStyleSheet(labelStyle);
    ui->labelMarca->setStyleSheet(labelStyle);
    ui->labelProveedor->setStyleSheet(labelStyle);
    ui->labelStockMinimo->setStyleSheet(labelStyle);
    ui->labelIVA->setStyleSheet(labelStyle);
    ui->labelActivo->setStyleSheet(labelStyle);

    QString btnStyle = R"(
        QPushButton {
            background-color: #007acc;
            color: white;
            font-weight: bold;
            border-radius: 6px;
            padding: 6px 12px;
            border: none;
        }
        QPushButton:hover {
            background-color: #005f99;
        }
        QPushButton:pressed {
            background-color: #004466;
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

    // Estilo para QPlainTextEdit
    ui->txtDescripcion->setStyleSheet("background-color: #ffffff; color: #000000; border:1px solid #ccc; border-radius:4px; padding:4px;");

    // Estilo para QComboBox
    QString comboStyle = "background-color: #ffffff; color: #000000; border:1px solid #ccc; border-radius:4px; padding:4px;";
    ui->cbMarca->setStyleSheet(comboStyle);
    ui->cbProveedor->setStyleSheet(comboStyle);

    // Estilo para QCheckBox
    ui->chkActivo->setStyleSheet("color: #333333; font-weight: bold;");

    ui->txtID->setEnabled(false);

    // ------------------------------ Configuración tabla ------------------------------
    ui->tableProductos->setStyleSheet(R"(
        QTableView {
            background-color: #ffffff;
            alternate-background-color: #f9f9f9;
            gridline-color: #dddddd;
            selection-background-color: #007acc;
            selection-color: white;
            border: 1px solid #cccccc;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #e0e0e0;
            font-weight: bold;
            border: 1px solid #cccccc;
            padding: 6px;
        }
        QTableView::item {
            padding: 4px;
            border-bottom: 1px solid #eeeeee;
        }
        QTableView::item:selected {
            background-color: #007acc;
            color: white;
        }
    )");

    // Primero cargamos combos y datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tableProductos->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Productos::filaSeleccionada);
}

Productos::~Productos()
{
    delete ui;
}

void Productos::cargarCombos()
{
    ui->cbMarca->clear();
    QSqlQuery query("SELECT id, nombre FROM marca WHERE activo = true ORDER BY nombre");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbMarca->addItem(nombre, id);
    }

    ui->cbProveedor->clear();
    query.exec("SELECT id, razon_social FROM proveedor WHERE activo = true ORDER BY razon_social");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString razonSocial = query.value(1).toString();
        ui->cbProveedor->addItem(razonSocial, id);
    }
}

void Productos::cargarDatos()
{
    QString sql = R"(
        SELECT p.id,
               p.codigo,
               p.nombre,
               p.descripcion,
               p.precio_venta,
               p.precio_compra,
               m.nombre AS marca,
               prov.razon_social AS proveedor,
               p.stock_minimo,
               CASE WHEN p.activo THEN 'Sí' ELSE 'No' END AS activo,
               p.iva
        FROM producto p
        LEFT JOIN marca m ON p.marca_id = m.id
        LEFT JOIN proveedor prov ON p.proveedor_id = prov.id
        ORDER BY p.id
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Código"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Nombre"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Descripción"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("P.Venta"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("P.Compra"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Marca"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Proveedor"));
    modelo->setHeaderData(8, Qt::Horizontal, tr("Stock Min"));
    modelo->setHeaderData(9, Qt::Horizontal, tr("Activo"));
    modelo->setHeaderData(10, Qt::Horizontal, tr("IVA"));

    ui->tableProductos->setModel(modelo);
    ui->tableProductos->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableProductos->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableProductos->horizontalHeader()->setStretchLastSection(true);
    ui->tableProductos->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Productos::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtCodigo->clear();
    ui->txtNombre->clear();
    ui->txtDescripcion->clear();
    ui->txtPrecioVenta->clear();
    ui->txtPrecioCompra->clear();
    ui->txtStockMinimo->clear();
    ui->txtIVA->setText("0.10");
    if(ui->cbMarca->count() > 0)
        ui->cbMarca->setCurrentIndex(0);
    if(ui->cbProveedor->count() > 0)
        ui->cbProveedor->setCurrentIndex(0);
    ui->chkActivo->setChecked(true);
    ui->txtCodigo->setFocus();
}

void Productos::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Productos::on_btnGuardar_clicked()
{
    QString codigo = ui->txtCodigo->text().trimmed();
    QString nombre = ui->txtNombre->text().trimmed();
    QString descripcion = ui->txtDescripcion->toPlainText().trimmed();
    double precioVenta = ui->txtPrecioVenta->text().toDouble();
    double precioCompra = ui->txtPrecioCompra->text().toDouble();
    int marcaId = ui->cbMarca->currentData().toInt();
    int proveedorId = ui->cbProveedor->currentData().toInt();
    int stockMinimo = ui->txtStockMinimo->text().toInt();
    bool activo = ui->chkActivo->isChecked();
    double iva = ui->txtIVA->text().toDouble();

    if(codigo.isEmpty() || nombre.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Código y Nombre son obligatorios."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO producto (codigo, nombre, descripcion, precio_venta, precio_compra, marca_id, proveedor_id, stock_minimo, activo, iva) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(codigo);
    query.addBindValue(nombre);
    query.addBindValue(descripcion);
    query.addBindValue(precioVenta);
    query.addBindValue(precioCompra);
    query.addBindValue(marcaId);
    query.addBindValue(proveedorId);
    query.addBindValue(stockMinimo);
    query.addBindValue(activo);
    query.addBindValue(iva);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el producto:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Producto guardado correctamente."));
}

void Productos::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    QString codigo = ui->txtCodigo->text().trimmed();
    QString nombre = ui->txtNombre->text().trimmed();
    QString descripcion = ui->txtDescripcion->toPlainText().trimmed();
    double precioVenta = ui->txtPrecioVenta->text().toDouble();
    double precioCompra = ui->txtPrecioCompra->text().toDouble();
    int marcaId = ui->cbMarca->currentData().toInt();
    int proveedorId = ui->cbProveedor->currentData().toInt();
    int stockMinimo = ui->txtStockMinimo->text().toInt();
    bool activo = ui->chkActivo->isChecked();
    double iva = ui->txtIVA->text().toDouble();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un producto para modificar."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE producto SET codigo=?, nombre=?, descripcion=?, precio_venta=?, precio_compra=?, marca_id=?, proveedor_id=?, stock_minimo=?, activo=?, iva=? WHERE id=?");
    query.addBindValue(codigo);
    query.addBindValue(nombre);
    query.addBindValue(descripcion);
    query.addBindValue(precioVenta);
    query.addBindValue(precioCompra);
    query.addBindValue(marcaId);
    query.addBindValue(proveedorId);
    query.addBindValue(stockMinimo);
    query.addBindValue(activo);
    query.addBindValue(iva);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el producto:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Producto modificado correctamente."));
}

void Productos::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un producto para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este producto?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM producto WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el producto:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Producto eliminado correctamente."));
}

void Productos::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());
    ui->txtCodigo->setText(modelo->data(modelo->index(row,1)).toString());
    ui->txtNombre->setText(modelo->data(modelo->index(row,2)).toString());
    ui->txtDescripcion->setPlainText(modelo->data(modelo->index(row,3)).toString());
    ui->txtPrecioVenta->setText(modelo->data(modelo->index(row,4)).toString());
    ui->txtPrecioCompra->setText(modelo->data(modelo->index(row,5)).toString());

    QString marcaNombre = modelo->data(modelo->index(row,6)).toString();
    int indexMarca = ui->cbMarca->findText(marcaNombre);
    if(indexMarca >= 0) ui->cbMarca->setCurrentIndex(indexMarca);

    QString proveedorNombre = modelo->data(modelo->index(row,7)).toString();
    int indexProveedor = ui->cbProveedor->findText(proveedorNombre);
    if(indexProveedor >= 0) ui->cbProveedor->setCurrentIndex(indexProveedor);

    ui->txtStockMinimo->setText(modelo->data(modelo->index(row,8)).toString());
    ui->chkActivo->setChecked(modelo->data(modelo->index(row,9)).toString() == "Sí");
    ui->txtIVA->setText(modelo->data(modelo->index(row,10)).toString());
}

void Productos::on_btnSalir_clicked()
{
    close();
}
