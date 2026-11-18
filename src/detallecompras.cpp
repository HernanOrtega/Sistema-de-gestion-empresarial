#include "detallecompras.h"
#include "ui_detallecompras.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QIntValidator>
#include <QDoubleValidator>

Detallecompras::Detallecompras(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Detallecompras),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo rojo moderno ------------------------------
    this->setStyleSheet("background-color: #fff0f0;"); // Fondo rojo claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ff6666; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtCantidad->setStyleSheet(lineEditStyle);
    ui->txtPrecioUnitario->setStyleSheet(lineEditStyle);
    ui->txtIVA->setStyleSheet(lineEditStyle);
    ui->txtSubtotal->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #cc0000; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelCompra->setStyleSheet(labelStyle);
    ui->labelProducto->setStyleSheet(labelStyle);
    ui->labelCantidad->setStyleSheet(labelStyle);
    ui->labelPrecioUnitario->setStyleSheet(labelStyle);
    ui->labelIVA->setStyleSheet(labelStyle);
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
    ui->cbCompra->setStyleSheet(comboStyle);
    ui->cbProducto->setStyleSheet(comboStyle);

    ui->txtID->setEnabled(false);
    ui->txtSubtotal->setEnabled(false); // Se calcula automáticamente

    // Configurar validación para números
    ui->txtCantidad->setValidator(new QIntValidator(1, 999999, this));
    ui->txtPrecioUnitario->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));
    ui->txtIVA->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // Conectar señales para cálculo automático del subtotal
    connect(ui->txtCantidad, &QLineEdit::textChanged, this, [this](const QString &) {
        calcularSubtotal();
    });
    connect(ui->txtPrecioUnitario, &QLineEdit::textChanged, this, [this](const QString &) {
        calcularSubtotal();
    });
    connect(ui->txtIVA, &QLineEdit::textChanged, this, [this](const QString &) {
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

    ui->tablaDetalleCompras->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaDetalleCompras->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Detallecompras::filaSeleccionada);
}

Detallecompras::~Detallecompras()
{
    delete ui;
}

void Detallecompras::calcularSubtotal()
{
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double iva = ui->txtIVA->text().toDouble();

    double subtotal = (cantidad * precioUnitario) + iva;
    ui->txtSubtotal->setText(QString::number(subtotal, 'f', 2));
}

void Detallecompras::cargarCombos()
{
    // Cargar compras activas
    ui->cbCompra->clear();
    QSqlQuery query;

    QString compraQuery = "SELECT c.id, 'Compra #' || c.id || ' - Proveedor: ' || COALESCE(p.razon_social, 'N/A') || ' - Total: ' || c.total || ' - Estado: ' || c.estado "
                          "FROM compra c "
                          "LEFT JOIN proveedor p ON c.proveedor_id = p.id "
                          "WHERE c.estado != 'ANULADA' "
                          "ORDER BY c.fecha_compra DESC";

    if(query.exec(compraQuery)) {
        while(query.next()) {
            int id = query.value(0).toInt();
            QString descripcion = query.value(1).toString();
            ui->cbCompra->addItem(descripcion, id);
        }
    } else {
        qDebug() << "Error cargando compras:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar las compras:\n%1").arg(query.lastError().text()));
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

void Detallecompras::cargarDatos()
{
    QString sql = "SELECT cd.id, "
                  "'Compra #' || c.id || ' - Proveedor: ' || COALESCE(p.razon_social, 'N/A') AS compra, "
                  "pr.nombre AS producto, "
                  "cd.cantidad, "
                  "cd.precio_unitario, "
                  "cd.iva, "
                  "cd.subtotal "
                  "FROM compra_detalle cd "
                  "LEFT JOIN compra c ON cd.compra_id = c.id "
                  "LEFT JOIN proveedor p ON c.proveedor_id = p.id "
                  "LEFT JOIN producto pr ON cd.producto_id = pr.id "
                  "ORDER BY c.fecha_compra DESC, cd.id";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Compra"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Producto"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Cantidad"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Precio Unitario"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("IVA"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Subtotal"));

    ui->tablaDetalleCompras->setModel(modelo);
    ui->tablaDetalleCompras->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaDetalleCompras->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaDetalleCompras->horizontalHeader()->setStretchLastSection(true);
    ui->tablaDetalleCompras->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Detallecompras::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtCantidad->clear();
    ui->txtPrecioUnitario->clear();
    ui->txtIVA->clear();
    ui->txtSubtotal->clear();

    if(ui->cbCompra->count() > 0)
        ui->cbCompra->setCurrentIndex(0);
    if(ui->cbProducto->count() > 0)
        ui->cbProducto->setCurrentIndex(0);

    ui->cbCompra->setFocus();
}

void Detallecompras::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Detallecompras::on_btnGuardar_clicked()
{
    int compraId = ui->cbCompra->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double iva = ui->txtIVA->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(compraId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una compra."));
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

    // Verificar si ya existe un detalle para esta compra y producto
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM compra_detalle WHERE compra_id = ? AND producto_id = ?");
    checkQuery.addBindValue(compraId);
    checkQuery.addBindValue(productoId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe un detalle para este producto en la compra seleccionada."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO compra_detalle (compra_id, producto_id, cantidad, precio_unitario, iva, subtotal) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(compraId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(iva);
    query.addBindValue(subtotal);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el detalle de la compra:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Detalle de la compra guardado correctamente."));
}

void Detallecompras::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int compraId = ui->cbCompra->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double iva = ui->txtIVA->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle para modificar."));
        return;
    }

    if(compraId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una compra."));
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

    // Verificar si ya existe otro detalle para esta compra y producto (excluyendo el actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM compra_detalle WHERE compra_id = ? AND producto_id = ? AND id != ?");
    checkQuery.addBindValue(compraId);
    checkQuery.addBindValue(productoId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otro detalle para este producto en la compra seleccionada."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE compra_detalle SET compra_id=?, producto_id=?, cantidad=?, precio_unitario=?, iva=?, subtotal=? WHERE id=?");
    query.addBindValue(compraId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(iva);
    query.addBindValue(subtotal);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el detalle de la compra:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Detalle de la compra modificado correctamente."));
}

void Detallecompras::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este detalle de la compra?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM compra_detalle WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el detalle de la compra:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Detalle de la compra eliminado correctamente."));
}

void Detallecompras::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
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
    query.prepare("SELECT compra_id, producto_id, cantidad, precio_unitario, iva, subtotal "
                  "FROM compra_detalle WHERE id = ?");
    query.addBindValue(idDetalle);

    if(query.exec() && query.next()) {
        int compraId = query.value(0).toInt();
        int productoId = query.value(1).toInt();
        int cantidad = query.value(2).toInt();
        double precioUnitario = query.value(3).toDouble();
        double iva = query.value(4).toDouble();
        double subtotal = query.value(5).toDouble();

        // Buscar por ID en lugar de por texto
        int indexCompra = ui->cbCompra->findData(compraId);
        if(indexCompra >= 0) {
            ui->cbCompra->setCurrentIndex(indexCompra);
        }

        int indexProducto = ui->cbProducto->findData(productoId);
        if(indexProducto >= 0) {
            ui->cbProducto->setCurrentIndex(indexProducto);
        }

        ui->txtCantidad->setText(QString::number(cantidad));
        ui->txtPrecioUnitario->setText(QString::number(precioUnitario, 'f', 2));
        ui->txtIVA->setText(QString::number(iva, 'f', 2));
        ui->txtSubtotal->setText(QString::number(subtotal, 'f', 2));
    } else {
        qDebug() << "Error al cargar datos del detalle:" << query.lastError().text();
    }
}

void Detallecompras::on_btnSalir_clicked()
{
    close();
}
