#include "detallepresupuestos.h"
#include "ui_detallepresupuestos.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QIntValidator>
#include <QDoubleValidator>

Detallepresupuestos::Detallepresupuestos(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Detallepresupuestos),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo naranja moderno ------------------------------
    this->setStyleSheet("background-color: #fff8f0;"); // Fondo naranja claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb366; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtCantidad->setStyleSheet(lineEditStyle);
    ui->txtPrecioUnitario->setStyleSheet(lineEditStyle);
    ui->txtSubtotal->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #cc5500; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelPresupuesto->setStyleSheet(labelStyle);
    ui->labelProducto->setStyleSheet(labelStyle);
    ui->labelCantidad->setStyleSheet(labelStyle);
    ui->labelPrecioUnitario->setStyleSheet(labelStyle);
    ui->labelSubtotal->setStyleSheet(labelStyle);

    // Estilo para botones en tono naranja
    QString btnStyle = R"(
        QPushButton {
            background-color: #ff8c00;
            color: #ffffff;
            font-weight: bold;
            border-radius: 6px;
            padding: 6px 12px;
            border: none;
        }
        QPushButton:hover {
            background-color: #ff7700;
        }
        QPushButton:pressed {
            background-color: #e67300;
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
    QString comboStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb366; border-radius: 4px; padding: 4px;";
    ui->cbPresupuesto->setStyleSheet(comboStyle);
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

    // ------------------------------ Configuración tabla en tonos naranjas ------------------------------
    ui->tablaDetallePresupuestos->setStyleSheet(R"(
        QTableView {
            background-color: #fffaf0;
            alternate-background-color: #fff5e6;
            gridline-color: #ffb366;
            selection-background-color: #ff8c00;
            selection-color: white;
            border: 1px solid #ffb366;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #e67300;
            color: white;
            font-weight: bold;
            border: 1px solid #cc5500;
            padding: 6px;
        }
        QTableView::item {
            padding: 4px;
            border-bottom: 1px solid #ffd9b3;
            color: #333333;
        }
        QTableView::item:selected {
            background-color: #ff8c00;
            color: white;
        }
    )");

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaDetallePresupuestos->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Detallepresupuestos::filaSeleccionada);
}

Detallepresupuestos::~Detallepresupuestos()
{
    delete ui;
}

void Detallepresupuestos::calcularSubtotal()
{
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();

    double subtotal = cantidad * precioUnitario;
    ui->txtSubtotal->setText(QString::number(subtotal, 'f', 2));
}

void Detallepresupuestos::cargarCombos()
{
    // Cargar presupuestos activos
    ui->cbPresupuesto->clear();
    QSqlQuery query;

    QString presupuestoQuery = "SELECT p.id, 'Presupuesto #' || p.id || ' - Cliente: ' || COALESCE(c.nombre_apellido_cliente, 'N/A') || ' - Total: ' || p.total || ' - Estado: ' || p.estado "
                               "FROM presupuesto p "
                               "LEFT JOIN cliente c ON p.cliente_id = c.id "
                               "WHERE p.estado != 'RECHAZADO' AND p.estado != 'VENCIDO' "
                               "ORDER BY p.fecha_presupuesto DESC";

    if(query.exec(presupuestoQuery)) {
        while(query.next()) {
            int id = query.value(0).toInt();
            QString descripcion = query.value(1).toString();
            ui->cbPresupuesto->addItem(descripcion, id);
        }
    } else {
        qDebug() << "Error cargando presupuestos:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar los presupuestos:\n%1").arg(query.lastError().text()));
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

void Detallepresupuestos::cargarDatos()
{
    QString sql = "SELECT pd.id, "
                  "'Presupuesto #' || p.id || ' - Cliente: ' || COALESCE(c.nombre_apellido_cliente, 'N/A') AS presupuesto, "
                  "pr.nombre AS producto, "
                  "pd.cantidad, "
                  "pd.precio_unitario, "
                  "pd.subtotal "
                  "FROM presupuesto_detalle pd "
                  "LEFT JOIN presupuesto p ON pd.presupuesto_id = p.id "
                  "LEFT JOIN cliente c ON p.cliente_id = c.id "
                  "LEFT JOIN producto pr ON pd.producto_id = pr.id "
                  "ORDER BY p.fecha_presupuesto DESC, pd.id";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Presupuesto"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Producto"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Cantidad"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Precio Unitario"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Subtotal"));

    ui->tablaDetallePresupuestos->setModel(modelo);
    ui->tablaDetallePresupuestos->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaDetallePresupuestos->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaDetallePresupuestos->horizontalHeader()->setStretchLastSection(true);
    ui->tablaDetallePresupuestos->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Detallepresupuestos::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtCantidad->clear();
    ui->txtPrecioUnitario->clear();
    ui->txtSubtotal->clear();

    if(ui->cbPresupuesto->count() > 0)
        ui->cbPresupuesto->setCurrentIndex(0);
    if(ui->cbProducto->count() > 0)
        ui->cbProducto->setCurrentIndex(0);

    ui->cbPresupuesto->setFocus();
}

void Detallepresupuestos::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Detallepresupuestos::on_btnGuardar_clicked()
{
    int presupuestoId = ui->cbPresupuesto->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(presupuestoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un presupuesto."));
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

    // Verificar si ya existe un detalle para este presupuesto y producto
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM presupuesto_detalle WHERE presupuesto_id = ? AND producto_id = ?");
    checkQuery.addBindValue(presupuestoId);
    checkQuery.addBindValue(productoId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe un detalle para este producto en el presupuesto seleccionado."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO presupuesto_detalle (presupuesto_id, producto_id, cantidad, precio_unitario, subtotal) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(presupuestoId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(subtotal);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el detalle del presupuesto:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Detalle del presupuesto guardado correctamente."));
}

void Detallepresupuestos::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int presupuestoId = ui->cbPresupuesto->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    double precioUnitario = ui->txtPrecioUnitario->text().toDouble();
    double subtotal = ui->txtSubtotal->text().toDouble();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle para modificar."));
        return;
    }

    if(presupuestoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un presupuesto."));
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

    // Verificar si ya existe otro detalle para este presupuesto y producto (excluyendo el actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM presupuesto_detalle WHERE presupuesto_id = ? AND producto_id = ? AND id != ?");
    checkQuery.addBindValue(presupuestoId);
    checkQuery.addBindValue(productoId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otro detalle para este producto en el presupuesto seleccionado."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE presupuesto_detalle SET presupuesto_id=?, producto_id=?, cantidad=?, precio_unitario=?, subtotal=? WHERE id=?");
    query.addBindValue(presupuestoId);
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(precioUnitario);
    query.addBindValue(subtotal);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el detalle del presupuesto:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Detalle del presupuesto modificado correctamente."));
}

void Detallepresupuestos::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este detalle del presupuesto?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM presupuesto_detalle WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el detalle del presupuesto:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Detalle del presupuesto eliminado correctamente."));
}

void Detallepresupuestos::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
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
    query.prepare("SELECT presupuesto_id, producto_id, cantidad, precio_unitario, subtotal "
                  "FROM presupuesto_detalle WHERE id = ?");
    query.addBindValue(idDetalle);

    if(query.exec() && query.next()) {
        int presupuestoId = query.value(0).toInt();
        int productoId = query.value(1).toInt();
        int cantidad = query.value(2).toInt();
        double precioUnitario = query.value(3).toDouble();
        double subtotal = query.value(4).toDouble();

        // Buscar por ID en lugar de por texto
        int indexPresupuesto = ui->cbPresupuesto->findData(presupuestoId);
        if(indexPresupuesto >= 0) {
            ui->cbPresupuesto->setCurrentIndex(indexPresupuesto);
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

void Detallepresupuestos::on_btnSalir_clicked()
{
    close();
}
