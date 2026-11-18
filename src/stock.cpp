#include "stock.h"
#include "ui_stock.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QIntValidator>

Stock::Stock(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Stock),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo cian moderno ------------------------------
    this->setStyleSheet("background-color: #f0fdff;"); // Fondo cian claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtCantidad->setStyleSheet(lineEditStyle);
    ui->txtCantidadMinima->setStyleSheet(lineEditStyle);
    ui->txtCantidadMaxima->setStyleSheet(lineEditStyle);
    ui->txtUbicacion->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #00838f; font-weight: bold; font-size: 14px;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelProducto->setStyleSheet(labelStyle);
    ui->labelCantidad->setStyleSheet(labelStyle);
    ui->labelCantidadMinima->setStyleSheet(labelStyle);
    ui->labelCantidadMaxima->setStyleSheet(labelStyle);
    ui->labelUbicacion->setStyleSheet(labelStyle);
    ui->labelFechaActualizacion->setStyleSheet(labelStyle);

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
    ui->cbProducto->setStyleSheet(comboStyle);

    // Estilo para QDateTimeEdit
    QString dateStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->dtFechaActualizacion->setStyleSheet(dateStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaActualizacion->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaActualizacion->setDateTime(QDateTime::currentDateTime());

    // Configurar validación para cantidades (solo números enteros)
    ui->txtCantidad->setValidator(new QIntValidator(0, 999999, this));
    ui->txtCantidadMinima->setValidator(new QIntValidator(0, 999999, this));
    ui->txtCantidadMaxima->setValidator(new QIntValidator(0, 999999, this));

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

    ui->tablaStock->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    if (ui->tablaStock->selectionModel()) {
        connect(ui->tablaStock->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                &Stock::filaSeleccionada);
    }
}

Stock::~Stock()
{
    delete ui;
}

void Stock::cargarCombos()
{
    // Cargar productos activos
    ui->cbProducto->clear();
    QSqlQuery query;
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

void Stock::cargarDatos()
{
    QString sql = "SELECT s.id, "
                  "p.nombre || ' (' || p.codigo || ')' AS producto, "
                  "s.cantidad, "
                  "s.cantidad_minima, "
                  "s.cantidad_maxima, "
                  "COALESCE(s.ubicacion, 'Sin ubicación') AS ubicacion, "
                  "TO_CHAR(s.fecha_actualizacion, 'YYYY-MM-DD HH24:MI') AS fecha_actualizacion "
                  "FROM stock s "
                  "LEFT JOIN producto p ON s.producto_id = p.id "
                  "ORDER BY p.nombre, s.fecha_actualizacion DESC";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Producto"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Cantidad"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Mínimo"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Máximo"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Ubicación"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Última Actualización"));

    ui->tablaStock->setModel(modelo);
    ui->tablaStock->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaStock->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaStock->horizontalHeader()->setStretchLastSection(true);
    ui->tablaStock->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Ajustar el ancho de las columnas
    ui->tablaStock->setColumnWidth(0, 50);   // ID
    ui->tablaStock->setColumnWidth(1, 250);  // Producto
    ui->tablaStock->setColumnWidth(2, 80);   // Cantidad
    ui->tablaStock->setColumnWidth(3, 70);   // Mínimo
    ui->tablaStock->setColumnWidth(4, 70);   // Máximo
    ui->tablaStock->setColumnWidth(5, 150);  // Ubicación
    ui->tablaStock->setColumnWidth(6, 150);  // Fecha Actualización

    limpiarCampos();
}

void Stock::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtCantidad->clear();
    ui->txtCantidadMinima->clear();
    ui->txtCantidadMaxima->clear();
    ui->txtUbicacion->clear();

    if(ui->cbProducto->count() > 0)
        ui->cbProducto->setCurrentIndex(0);

    ui->dtFechaActualizacion->setDateTime(QDateTime::currentDateTime());

    ui->cbProducto->setFocus();
}

void Stock::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Stock::on_btnGuardar_clicked()
{
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    int cantidadMinima = ui->txtCantidadMinima->text().toInt();
    int cantidadMaxima = ui->txtCantidadMaxima->text().toInt();
    QString ubicacion = ui->txtUbicacion->text().trimmed();
    QDateTime fechaActualizacion = ui->dtFechaActualizacion->dateTime();

    if(productoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un producto."));
        return;
    }

    if(cantidad < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad no puede ser negativa."));
        return;
    }

    if(cantidadMinima < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad mínima no puede ser negativa."));
        return;
    }

    if(cantidadMaxima < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad máxima no puede ser negativa."));
        return;
    }

    if(cantidadMaxima > 0 && cantidadMinima > cantidadMaxima) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad mínima no puede ser mayor que la cantidad máxima."));
        return;
    }

    if(cantidadMaxima > 0 && cantidad > cantidadMaxima) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad actual no puede ser mayor que la cantidad máxima."));
        return;
    }

    // Verificar si ya existe stock para este producto
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM stock WHERE producto_id = ?");
    checkQuery.addBindValue(productoId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe un registro de stock para este producto."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO stock (producto_id, cantidad, cantidad_minima, cantidad_maxima, ubicacion, fecha_actualizacion) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(cantidadMinima);
    query.addBindValue(cantidadMaxima);
    query.addBindValue(ubicacion.isEmpty() ? QVariant() : QVariant(ubicacion));
    query.addBindValue(fechaActualizacion);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el registro de stock:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Registro de stock guardado correctamente."));
}

void Stock::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidad = ui->txtCantidad->text().toInt();
    int cantidadMinima = ui->txtCantidadMinima->text().toInt();
    int cantidadMaxima = ui->txtCantidadMaxima->text().toInt();
    QString ubicacion = ui->txtUbicacion->text().trimmed();
    QDateTime fechaActualizacion = ui->dtFechaActualizacion->dateTime();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un registro de stock para modificar."));
        return;
    }

    if(productoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un producto."));
        return;
    }

    if(cantidad < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad no puede ser negativa."));
        return;
    }

    if(cantidadMinima < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad mínima no puede ser negativa."));
        return;
    }

    if(cantidadMaxima < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad máxima no puede ser negativa."));
        return;
    }

    if(cantidadMaxima > 0 && cantidadMinima > cantidadMaxima) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad mínima no puede ser mayor que la cantidad máxima."));
        return;
    }

    if(cantidadMaxima > 0 && cantidad > cantidadMaxima) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad actual no puede ser mayor que la cantidad máxima."));
        return;
    }

    // Verificar si ya existe otro registro de stock para este producto (excluyendo el actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM stock WHERE producto_id = ? AND id != ?");
    checkQuery.addBindValue(productoId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otro registro de stock para este producto."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE stock SET producto_id=?, cantidad=?, cantidad_minima=?, cantidad_maxima=?, ubicacion=?, fecha_actualizacion=? WHERE id=?");
    query.addBindValue(productoId);
    query.addBindValue(cantidad);
    query.addBindValue(cantidadMinima);
    query.addBindValue(cantidadMaxima);
    query.addBindValue(ubicacion.isEmpty() ? QVariant() : QVariant(ubicacion));
    query.addBindValue(fechaActualizacion);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el registro de stock:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Registro de stock modificado correctamente."));
}

void Stock::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un registro de stock para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este registro de stock?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM stock WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el registro de stock:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Registro de stock eliminado correctamente."));
}

void Stock::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los IDs directamente de la base de datos para una búsqueda precisa
    int idStock = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT producto_id, cantidad, cantidad_minima, cantidad_maxima, ubicacion, fecha_actualizacion "
                  "FROM stock WHERE id = ?");
    query.addBindValue(idStock);

    if(query.exec() && query.next()) {
        int productoId = query.value(0).toInt();
        int cantidad = query.value(1).toInt();
        int cantidadMinima = query.value(2).toInt();
        int cantidadMaxima = query.value(3).toInt();
        QString ubicacion = query.value(4).toString();
        QDateTime fechaActualizacion = query.value(5).toDateTime();

        // Buscar por ID en lugar de por texto
        int indexProducto = ui->cbProducto->findData(productoId);
        if(indexProducto >= 0) {
            ui->cbProducto->setCurrentIndex(indexProducto);
        }

        ui->txtCantidad->setText(QString::number(cantidad));
        ui->txtCantidadMinima->setText(QString::number(cantidadMinima));
        ui->txtCantidadMaxima->setText(QString::number(cantidadMaxima));
        ui->txtUbicacion->setText(ubicacion);

        if(fechaActualizacion.isValid()) {
            ui->dtFechaActualizacion->setDateTime(fechaActualizacion);
        }
    } else {
        qDebug() << "Error al cargar datos del stock:" << query.lastError().text();
    }
}

void Stock::on_btnSalir_clicked()
{
    close();
}
