#include "cotizaciones.h"
#include "ui_cotizaciones.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QDoubleValidator>

Cotizaciones::Cotizaciones(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Cotizaciones),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo naranja moderno ------------------------------
    this->setStyleSheet("background-color: #fff8f0;"); // Fondo naranja claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb366; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtCotizacionCompra->setStyleSheet(lineEditStyle);
    ui->txtCotizacionVenta->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #cc5500; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelMoneda->setStyleSheet(labelStyle);
    ui->labelCotizacionCompra->setStyleSheet(labelStyle);
    ui->labelCotizacionVenta->setStyleSheet(labelStyle);
    ui->labelFechaCotizacion->setStyleSheet(labelStyle);
    ui->labelUsuario->setStyleSheet(labelStyle);

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
    ui->cbMoneda->setStyleSheet(comboStyle);
    ui->cbUsuario->setStyleSheet(comboStyle);

    // Estilo para QDateEdit
    QString dateStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb366; border-radius: 4px; padding: 4px;";
    ui->deFechaCotizacion->setStyleSheet(dateStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->deFechaCotizacion->setDisplayFormat("yyyy-MM-dd");
    ui->deFechaCotizacion->setDate(QDate::currentDate());

    // Configurar validación para montos (solo números)
    ui->txtCotizacionCompra->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));
    ui->txtCotizacionVenta->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // ------------------------------ Configuración tabla en tonos naranjas ------------------------------
    ui->tablaCotizaciones->setStyleSheet(R"(
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
    connect(ui->tablaCotizaciones->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Cotizaciones::filaSeleccionada);
}

Cotizaciones::~Cotizaciones()
{
    delete ui;
}

void Cotizaciones::cargarCombos()
{
    // Cargar monedas
    ui->cbMoneda->clear();
    QSqlQuery query("SELECT id, nombre FROM moneda ORDER BY nombre");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbMoneda->addItem(nombre, id);
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

void Cotizaciones::cargarDatos()
{
    QString sql = R"(
        SELECT c.id,
               m.nombre AS moneda,
               c.cotizacion_compra,
               c.cotizacion_venta,
               TO_CHAR(c.fecha_cotizacion, 'YYYY-MM-DD') AS fecha_cotizacion,
               u.username AS usuario
        FROM cotizacion c
        LEFT JOIN moneda m ON c.moneda_id = m.id
        LEFT JOIN usuario u ON c.usuario_id = u.id
        ORDER BY c.fecha_cotizacion DESC, m.nombre
    )";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Moneda"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Cotización Compra"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Cotización Venta"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Fecha Cotización"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Usuario"));

    ui->tablaCotizaciones->setModel(modelo);
    ui->tablaCotizaciones->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaCotizaciones->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaCotizaciones->horizontalHeader()->setStretchLastSection(true);
    ui->tablaCotizaciones->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Cotizaciones::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtCotizacionCompra->clear();
    ui->txtCotizacionVenta->clear();

    if(ui->cbMoneda->count() > 0)
        ui->cbMoneda->setCurrentIndex(0);
    if(ui->cbUsuario->count() > 0)
        ui->cbUsuario->setCurrentIndex(0);

    ui->deFechaCotizacion->setDate(QDate::currentDate());

    ui->cbMoneda->setFocus();
}

void Cotizaciones::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Cotizaciones::on_btnGuardar_clicked()
{
    int monedaId = ui->cbMoneda->currentData().toInt();
    double cotizacionCompra = ui->txtCotizacionCompra->text().toDouble();
    double cotizacionVenta = ui->txtCotizacionVenta->text().toDouble();
    QDate fechaCotizacion = ui->deFechaCotizacion->date();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(monedaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una moneda."));
        return;
    }

    if(cotizacionCompra <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cotización de compra debe ser mayor a cero."));
        return;
    }

    if(cotizacionVenta <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cotización de venta debe ser mayor a cero."));
        return;
    }

    if(cotizacionVenta < cotizacionCompra) {
        QMessageBox::warning(this, tr("Atención"), tr("La cotización de venta no puede ser menor que la cotización de compra."));
        return;
    }

    if(fechaCotizacion > QDate::currentDate()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha de cotización no puede ser futura."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    // Verificar si ya existe una cotización para esta moneda en esta fecha
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM cotizacion WHERE moneda_id = ? AND fecha_cotizacion = ?");
    checkQuery.addBindValue(monedaId);
    checkQuery.addBindValue(fechaCotizacion);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe una cotización para esta moneda en la fecha seleccionada."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO cotizacion (moneda_id, cotizacion_compra, cotizacion_venta, fecha_cotizacion, usuario_id) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(monedaId);
    query.addBindValue(cotizacionCompra);
    query.addBindValue(cotizacionVenta);
    query.addBindValue(fechaCotizacion);
    query.addBindValue(usuarioId);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar la cotización:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Cotización guardada correctamente."));
}

void Cotizaciones::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int monedaId = ui->cbMoneda->currentData().toInt();
    double cotizacionCompra = ui->txtCotizacionCompra->text().toDouble();
    double cotizacionVenta = ui->txtCotizacionVenta->text().toDouble();
    QDate fechaCotizacion = ui->deFechaCotizacion->date();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una cotización para modificar."));
        return;
    }

    if(monedaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una moneda."));
        return;
    }

    if(cotizacionCompra <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cotización de compra debe ser mayor a cero."));
        return;
    }

    if(cotizacionVenta <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cotización de venta debe ser mayor a cero."));
        return;
    }

    if(cotizacionVenta < cotizacionCompra) {
        QMessageBox::warning(this, tr("Atención"), tr("La cotización de venta no puede ser menor que la cotización de compra."));
        return;
    }

    if(fechaCotizacion > QDate::currentDate()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha de cotización no puede ser futura."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    // Verificar si ya existe otra cotización para esta moneda en esta fecha (excluyendo la actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM cotizacion WHERE moneda_id = ? AND fecha_cotizacion = ? AND id != ?");
    checkQuery.addBindValue(monedaId);
    checkQuery.addBindValue(fechaCotizacion);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otra cotización para esta moneda en la fecha seleccionada."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE cotizacion SET moneda_id=?, cotizacion_compra=?, cotizacion_venta=?, fecha_cotizacion=?, usuario_id=? WHERE id=?");
    query.addBindValue(monedaId);
    query.addBindValue(cotizacionCompra);
    query.addBindValue(cotizacionVenta);
    query.addBindValue(fechaCotizacion);
    query.addBindValue(usuarioId);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar la cotización:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Cotización modificada correctamente."));
}

void Cotizaciones::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una cotización para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar esta cotización?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM cotizacion WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar la cotización:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Cotización eliminada correctamente."));
}

void Cotizaciones::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los IDs directamente de la base de datos para una búsqueda precisa
    int idCotizacion = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT moneda_id, cotizacion_compra, cotizacion_venta, fecha_cotizacion, usuario_id "
                  "FROM cotizacion WHERE id = ?");
    query.addBindValue(idCotizacion);

    if(query.exec() && query.next()) {
        int monedaId = query.value(0).toInt();
        double cotizacionCompra = query.value(1).toDouble();
        double cotizacionVenta = query.value(2).toDouble();
        QDate fechaCotizacion = query.value(3).toDate();
        int usuarioId = query.value(4).toInt();

        // Buscar por ID en lugar de por texto
        int indexMoneda = ui->cbMoneda->findData(monedaId);
        if(indexMoneda >= 0) {
            ui->cbMoneda->setCurrentIndex(indexMoneda);
        }

        ui->txtCotizacionCompra->setText(QString::number(cotizacionCompra, 'f', 2));
        ui->txtCotizacionVenta->setText(QString::number(cotizacionVenta, 'f', 2));

        if(fechaCotizacion.isValid()) {
            ui->deFechaCotizacion->setDate(fechaCotizacion);
        }

        int indexUsuario = ui->cbUsuario->findData(usuarioId);
        if(indexUsuario >= 0) {
            ui->cbUsuario->setCurrentIndex(indexUsuario);
        }
    } else {
        qDebug() << "Error al cargar datos de la cotización:" << query.lastError().text();
    }
}

void Cotizaciones::on_btnSalir_clicked()
{
    close();
}
