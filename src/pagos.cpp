#include "pagos.h"
#include "ui_pagos.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QDoubleValidator>

Pagos::Pagos(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Pagos),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo marrón moderno ------------------------------
    this->setStyleSheet("background-color: #f8f4f0;"); // Fondo marrón claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #a1887f; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtMontoTotal->setStyleSheet(lineEditStyle);
    ui->txtObservaciones->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #5d4037; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelProveedor->setStyleSheet(labelStyle);
    ui->labelMontoTotal->setStyleSheet(labelStyle);
    ui->labelFechaPago->setStyleSheet(labelStyle);
    ui->labelMoneda->setStyleSheet(labelStyle);
    ui->labelObservaciones->setStyleSheet(labelStyle);
    ui->labelUsuario->setStyleSheet(labelStyle);

    // Estilo para botones en tono marrón
    QString btnStyle = R"(
        QPushButton {
            background-color: #8d6e63;
            color: #ffffff;
            font-weight: bold;
            border-radius: 6px;
            padding: 6px 12px;
            border: none;
        }
        QPushButton:hover {
            background-color: #6d4c41;
        }
        QPushButton:pressed {
            background-color: #5d4037;
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
    QString comboStyle = "background-color: #ffffff; color: #000000; border: 1px solid #a1887f; border-radius: 4px; padding: 4px;";
    ui->cbProveedor->setStyleSheet(comboStyle);
    ui->cbMoneda->setStyleSheet(comboStyle);
    ui->cbUsuario->setStyleSheet(comboStyle);

    // Estilo para QDateTimeEdit
    QString dateTimeStyle = "background-color: #ffffff; color: #000000; border: 1px solid #a1887f; border-radius: 4px; padding: 4px;";
    ui->dtFechaPago->setStyleSheet(dateTimeStyle);

    // Estilo para QTextEdit
    QString textEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #a1887f; border-radius: 4px; padding: 4px;";
    ui->txtObservaciones->setStyleSheet(textEditStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaPago->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaPago->setDateTime(QDateTime::currentDateTime());

    // Configurar validación para montos (solo números)
    ui->txtMontoTotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // ------------------------------ Configuración tabla en tonos marrones ------------------------------
    QString tablaStyle = R"(
        QTableView {
            background-color: #faf6f2;
            alternate-background-color: #f0e6dc;
            gridline-color: #a1887f;
            selection-background-color: #8d6e63;
            selection-color: white;
            border: 1px solid #a1887f;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #6d4c41;
            color: white;
            font-weight: bold;
            border: 1px solid #5d4037;
            padding: 6px;
        }
        QTableView::item {
            padding: 4px;
            border-bottom: 1px solid #d7ccc8;
            color: #333333;
        }
        QTableView::item:selected {
            background-color: #8d6e63;
            color: white;
        }
    )";

    ui->tablaPagos->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaPagos->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Pagos::filaSeleccionada);
}

Pagos::~Pagos()
{
    delete ui;
}

void Pagos::cargarCombos()
{
    // Cargar proveedores activos
    ui->cbProveedor->clear();
    QSqlQuery query("SELECT id, razon_social FROM proveedor WHERE activo = true ORDER BY razon_social");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString razonSocial = query.value(1).toString();
        ui->cbProveedor->addItem(razonSocial, id);
    }

    // Cargar monedas
    ui->cbMoneda->clear();
    query.exec("SELECT id, nombre FROM moneda ORDER BY nombre");
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

void Pagos::cargarDatos()
{
    QString sql = R"(
        SELECT p.id,
               pr.razon_social AS proveedor,
               p.monto_total,
               TO_CHAR(p.fecha_pago, 'YYYY-MM-DD HH24:MI') AS fecha_pago,
               m.nombre AS moneda,
               p.observaciones,
               u.username AS usuario
        FROM pago p
        LEFT JOIN proveedor pr ON p.proveedor_id = pr.id
        LEFT JOIN moneda m ON p.moneda_id = m.id
        LEFT JOIN usuario u ON p.usuario_id = u.id
        ORDER BY p.fecha_pago DESC
    )";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Proveedor"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Monto Total"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Fecha Pago"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Moneda"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Observaciones"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Usuario"));

    ui->tablaPagos->setModel(modelo);
    ui->tablaPagos->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaPagos->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaPagos->horizontalHeader()->setStretchLastSection(true);
    ui->tablaPagos->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Pagos::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtMontoTotal->clear();
    ui->txtObservaciones->clear();

    if(ui->cbProveedor->count() > 0)
        ui->cbProveedor->setCurrentIndex(0);
    if(ui->cbMoneda->count() > 0)
        ui->cbMoneda->setCurrentIndex(0);
    if(ui->cbUsuario->count() > 0)
        ui->cbUsuario->setCurrentIndex(0);

    ui->dtFechaPago->setDateTime(QDateTime::currentDateTime());

    ui->cbProveedor->setFocus();
}

void Pagos::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Pagos::on_btnGuardar_clicked()
{
    int proveedorId = ui->cbProveedor->currentData().toInt();
    double montoTotal = ui->txtMontoTotal->text().toDouble();
    QDateTime fechaPago = ui->dtFechaPago->dateTime();
    int monedaId = ui->cbMoneda->currentData().toInt();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(proveedorId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un proveedor."));
        return;
    }

    if(montoTotal <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El monto total debe ser mayor a cero."));
        return;
    }

    if(monedaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una moneda."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO pago (proveedor_id, monto_total, fecha_pago, moneda_id, observaciones, usuario_id) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(proveedorId);
    query.addBindValue(montoTotal);
    query.addBindValue(fechaPago);
    query.addBindValue(monedaId);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el pago:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Pago guardado correctamente."));
}

void Pagos::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int proveedorId = ui->cbProveedor->currentData().toInt();
    double montoTotal = ui->txtMontoTotal->text().toDouble();
    QDateTime fechaPago = ui->dtFechaPago->dateTime();
    int monedaId = ui->cbMoneda->currentData().toInt();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pago para modificar."));
        return;
    }

    if(proveedorId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un proveedor."));
        return;
    }

    if(montoTotal <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El monto total debe ser mayor a cero."));
        return;
    }

    if(monedaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una moneda."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE pago SET proveedor_id=?, monto_total=?, fecha_pago=?, moneda_id=?, observaciones=?, usuario_id=? WHERE id=?");
    query.addBindValue(proveedorId);
    query.addBindValue(montoTotal);
    query.addBindValue(fechaPago);
    query.addBindValue(monedaId);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el pago:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Pago modificado correctamente."));
}

void Pagos::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un pago para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este pago?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM pago WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el pago:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Pago eliminado correctamente."));
}

void Pagos::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los IDs directamente de la base de datos para una búsqueda precisa
    int idPago = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT proveedor_id, monto_total, fecha_pago, moneda_id, observaciones, usuario_id "
                  "FROM pago WHERE id = ?");
    query.addBindValue(idPago);

    if(query.exec() && query.next()) {
        int proveedorId = query.value(0).toInt();
        double montoTotal = query.value(1).toDouble();
        QDateTime fechaPago = query.value(2).toDateTime();
        int monedaId = query.value(3).toInt();
        QString observaciones = query.value(4).toString();
        int usuarioId = query.value(5).toInt();

        // Buscar por ID en lugar de por texto
        int indexProveedor = ui->cbProveedor->findData(proveedorId);
        if(indexProveedor >= 0) {
            ui->cbProveedor->setCurrentIndex(indexProveedor);
        }

        ui->txtMontoTotal->setText(QString::number(montoTotal, 'f', 2));

        if(fechaPago.isValid()) {
            ui->dtFechaPago->setDateTime(fechaPago);
        }

        int indexMoneda = ui->cbMoneda->findData(monedaId);
        if(indexMoneda >= 0) {
            ui->cbMoneda->setCurrentIndex(indexMoneda);
        }

        ui->txtObservaciones->setPlainText(observaciones);

        int indexUsuario = ui->cbUsuario->findData(usuarioId);
        if(indexUsuario >= 0) {
            ui->cbUsuario->setCurrentIndex(indexUsuario);
        }
    } else {
        qDebug() << "Error al cargar datos del pago:" << query.lastError().text();
    }
}

void Pagos::on_btnSalir_clicked()
{
    close();
}
