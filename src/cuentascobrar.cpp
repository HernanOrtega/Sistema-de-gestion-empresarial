#include "cuentascobrar.h"
#include "ui_cuentascobrar.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QDoubleValidator>

Cuentascobrar::Cuentascobrar(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Cuentascobrar),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo morado moderno ------------------------------
    this->setStyleSheet("background-color: #f8f4ff;"); // Fondo morado claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ba68c8; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtMontoTotal->setStyleSheet(lineEditStyle);
    ui->txtSaldoPendiente->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #6a1b9a; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelCliente->setStyleSheet(labelStyle);
    ui->labelVenta->setStyleSheet(labelStyle);
    ui->labelMontoTotal->setStyleSheet(labelStyle);
    ui->labelSaldoPendiente->setStyleSheet(labelStyle);
    ui->labelFechaEmision->setStyleSheet(labelStyle);
    ui->labelFechaVencimiento->setStyleSheet(labelStyle);
    ui->labelEstado->setStyleSheet(labelStyle);

    // Estilo para botones en tono morado
    QString btnStyle = R"(
        QPushButton {
            background-color: #9c27b0;
            color: #ffffff;
            font-weight: bold;
            border-radius: 6px;
            padding: 6px 12px;
            border: none;
        }
        QPushButton:hover {
            background-color: #8e24aa;
        }
        QPushButton:pressed {
            background-color: #7b1fa2;
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
    QString comboStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ba68c8; border-radius: 4px; padding: 4px;";
    ui->cbCliente->setStyleSheet(comboStyle);
    ui->cbVenta->setStyleSheet(comboStyle);
    ui->cbEstado->setStyleSheet(comboStyle);

    // Estilo para QDateTimeEdit y QDateEdit
    QString dateStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ba68c8; border-radius: 4px; padding: 4px;";
    ui->dtFechaEmision->setStyleSheet(dateStyle);
    ui->deFechaVencimiento->setStyleSheet(dateStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaEmision->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaEmision->setDateTime(QDateTime::currentDateTime());

    ui->deFechaVencimiento->setDisplayFormat("yyyy-MM-dd");
    ui->deFechaVencimiento->setDate(QDate::currentDate().addDays(30)); // 30 días por defecto

    // Configurar combo de estados
    ui->cbEstado->clear();
    ui->cbEstado->addItem("PENDIENTE", "PENDIENTE");
    ui->cbEstado->addItem("PAGADA", "PAGADA");
    ui->cbEstado->addItem("VENCIDA", "VENCIDA");
    ui->cbEstado->addItem("CANCELADA", "CANCELADA");

    // Configurar validación para montos (solo números)
    ui->txtMontoTotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));
    ui->txtSaldoPendiente->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // ------------------------------ Configuración tabla en tonos morados ------------------------------
    QString tablaStyle = R"(
        QTableView {
            background-color: #faf5ff;
            alternate-background-color: #f3e5f5;
            gridline-color: #ba68c8;
            selection-background-color: #9c27b0;
            selection-color: white;
            border: 1px solid #ba68c8;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #7b1fa2;
            color: white;
            font-weight: bold;
            border: 1px solid #6a1b9a;
            padding: 6px;
        }
        QTableView::item {
            padding: 4px;
            border-bottom: 1px solid #e1bee7;
            color: #333333;
        }
        QTableView::item:selected {
            background-color: #9c27b0;
            color: white;
        }
    )";

    ui->tablaCuentasCobrar->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaCuentasCobrar->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Cuentascobrar::filaSeleccionada);
}

Cuentascobrar::~Cuentascobrar()
{
    delete ui;
}

void Cuentascobrar::cargarCombos()
{
    // Cargar clientes activos
    ui->cbCliente->clear();
    QSqlQuery query("SELECT id, nombre_apellido_cliente FROM cliente WHERE activo = true ORDER BY nombre_apellido_cliente");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbCliente->addItem(nombre, id);
    }

    // Cargar ventas activas - CORREGIDO para PostgreSQL
    ui->cbVenta->clear();

    // Consulta corregida usando || para concatenación en PostgreSQL
    QString ventaQuery = "SELECT v.id, "
                         "'Venta #' || v.id || ' - Cliente: ' || COALESCE(c.nombre_apellido_cliente, 'N/A') || ' - Total: ' || v.total || ' - Estado: ' || v.estado "
                         "FROM venta v "
                         "LEFT JOIN cliente c ON v.cliente_id = c.id "
                         "WHERE v.estado != 'ANULADA' "
                         "ORDER BY v.fecha_venta DESC";

    if(query.exec(ventaQuery)) {
        while(query.next()) {
            int id = query.value(0).toInt();
            QString descripcion = query.value(1).toString();
            ui->cbVenta->addItem(descripcion, id);
        }
    } else {
        qDebug() << "Error cargando ventas:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar las ventas:\n%1").arg(query.lastError().text()));
    }
}

void Cuentascobrar::cargarDatos()
{
    // CORREGIDO: Usar || en lugar de CONCAT para PostgreSQL
    QString sql = "SELECT cc.id, "
                  "c.nombre_apellido_cliente AS cliente, "
                  "'Venta #' || v.id || ' - Cliente: ' || COALESCE(c2.nombre_apellido_cliente, 'N/A') AS venta, "
                  "cc.monto_total, "
                  "cc.saldo_pendiente, "
                  "TO_CHAR(cc.fecha_emision, 'YYYY-MM-DD HH24:MI') AS fecha_emision, "
                  "TO_CHAR(cc.fecha_vencimiento, 'YYYY-MM-DD') AS fecha_vencimiento, "
                  "cc.estado "
                  "FROM cuenta_a_cobrar cc "
                  "LEFT JOIN cliente c ON cc.cliente_id = c.id "
                  "LEFT JOIN venta v ON cc.venta_id = v.id "
                  "LEFT JOIN cliente c2 ON v.cliente_id = c2.id "
                  "ORDER BY cc.fecha_emision DESC";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Cliente"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Venta"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Monto Total"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Saldo Pendiente"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Fecha Emisión"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Fecha Vencimiento"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Estado"));

    ui->tablaCuentasCobrar->setModel(modelo);
    ui->tablaCuentasCobrar->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaCuentasCobrar->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaCuentasCobrar->horizontalHeader()->setStretchLastSection(true);
    ui->tablaCuentasCobrar->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Cuentascobrar::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtMontoTotal->clear();
    ui->txtSaldoPendiente->clear();

    if(ui->cbCliente->count() > 0)
        ui->cbCliente->setCurrentIndex(0);
    if(ui->cbVenta->count() > 0)
        ui->cbVenta->setCurrentIndex(0);
    if(ui->cbEstado->count() > 0)
        ui->cbEstado->setCurrentIndex(0); // PENDIENTE por defecto

    ui->dtFechaEmision->setDateTime(QDateTime::currentDateTime());
    ui->deFechaVencimiento->setDate(QDate::currentDate().addDays(30));

    ui->cbCliente->setFocus();
}

void Cuentascobrar::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Cuentascobrar::on_btnGuardar_clicked()
{
    int clienteId = ui->cbCliente->currentData().toInt();
    int ventaId = ui->cbVenta->currentData().toInt();
    double montoTotal = ui->txtMontoTotal->text().toDouble();
    double saldoPendiente = ui->txtSaldoPendiente->text().toDouble();
    QDateTime fechaEmision = ui->dtFechaEmision->dateTime();
    QDate fechaVencimiento = ui->deFechaVencimiento->date();
    QString estado = ui->cbEstado->currentData().toString();

    if(clienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cliente."));
        return;
    }

    if(ventaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una venta."));
        return;
    }

    if(montoTotal <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El monto total debe ser mayor a cero."));
        return;
    }

    if(saldoPendiente < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El saldo pendiente no puede ser negativo."));
        return;
    }

    if(saldoPendiente > montoTotal) {
        QMessageBox::warning(this, tr("Atención"), tr("El saldo pendiente no puede ser mayor al monto total."));
        return;
    }

    if(fechaVencimiento < QDate::currentDate()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha de vencimiento no puede ser anterior a la fecha actual."));
        return;
    }

    // Verificar si ya existe una cuenta por cobrar para esta venta
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM cuenta_a_cobrar WHERE venta_id = ?");
    checkQuery.addBindValue(ventaId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe una cuenta por cobrar para esta venta."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO cuenta_a_cobrar (cliente_id, venta_id, monto_total, saldo_pendiente, fecha_emision, fecha_vencimiento, estado) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(clienteId);
    query.addBindValue(ventaId);
    query.addBindValue(montoTotal);
    query.addBindValue(saldoPendiente);
    query.addBindValue(fechaEmision);
    query.addBindValue(fechaVencimiento);
    query.addBindValue(estado);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar la cuenta por cobrar:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Cuenta por cobrar guardada correctamente."));
}

void Cuentascobrar::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int clienteId = ui->cbCliente->currentData().toInt();
    int ventaId = ui->cbVenta->currentData().toInt();
    double montoTotal = ui->txtMontoTotal->text().toDouble();
    double saldoPendiente = ui->txtSaldoPendiente->text().toDouble();
    QDateTime fechaEmision = ui->dtFechaEmision->dateTime();
    QDate fechaVencimiento = ui->deFechaVencimiento->date();
    QString estado = ui->cbEstado->currentData().toString();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una cuenta por cobrar para modificar."));
        return;
    }

    if(clienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cliente."));
        return;
    }

    if(ventaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una venta."));
        return;
    }

    if(montoTotal <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El monto total debe ser mayor a cero."));
        return;
    }

    if(saldoPendiente < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El saldo pendiente no puede ser negativo."));
        return;
    }

    if(saldoPendiente > montoTotal) {
        QMessageBox::warning(this, tr("Atención"), tr("El saldo pendiente no puede ser mayor al monto total."));
        return;
    }

    if(fechaVencimiento < QDate::currentDate()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha de vencimiento no puede ser anterior a la fecha actual."));
        return;
    }

    // Verificar si ya existe otra cuenta por cobrar para esta venta (excluyendo la actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM cuenta_a_cobrar WHERE venta_id = ? AND id != ?");
    checkQuery.addBindValue(ventaId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otra cuenta por cobrar para esta venta."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE cuenta_a_cobrar SET cliente_id=?, venta_id=?, monto_total=?, saldo_pendiente=?, fecha_emision=?, fecha_vencimiento=?, estado=? WHERE id=?");
    query.addBindValue(clienteId);
    query.addBindValue(ventaId);
    query.addBindValue(montoTotal);
    query.addBindValue(saldoPendiente);
    query.addBindValue(fechaEmision);
    query.addBindValue(fechaVencimiento);
    query.addBindValue(estado);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar la cuenta por cobrar:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Cuenta por cobrar modificada correctamente."));
}

void Cuentascobrar::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una cuenta por cobrar para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar esta cuenta por cobrar?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM cuenta_a_cobrar WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar la cuenta por cobrar:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Cuenta por cobrar eliminada correctamente."));
}

void Cuentascobrar::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los IDs directamente de la base de datos para una búsqueda precisa
    int idCuenta = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT cliente_id, venta_id, monto_total, saldo_pendiente, fecha_emision, fecha_vencimiento, estado "
                  "FROM cuenta_a_cobrar WHERE id = ?");
    query.addBindValue(idCuenta);

    if(query.exec() && query.next()) {
        int clienteId = query.value(0).toInt();
        int ventaId = query.value(1).toInt();
        double montoTotal = query.value(2).toDouble();
        double saldoPendiente = query.value(3).toDouble();
        QDateTime fechaEmision = query.value(4).toDateTime();
        QDate fechaVencimiento = query.value(5).toDate();
        QString estado = query.value(6).toString();

        // Buscar por ID en lugar de por texto
        int indexCliente = ui->cbCliente->findData(clienteId);
        if(indexCliente >= 0) {
            ui->cbCliente->setCurrentIndex(indexCliente);
        }

        int indexVenta = ui->cbVenta->findData(ventaId);
        if(indexVenta >= 0) {
            ui->cbVenta->setCurrentIndex(indexVenta);
        }

        ui->txtMontoTotal->setText(QString::number(montoTotal, 'f', 2));
        ui->txtSaldoPendiente->setText(QString::number(saldoPendiente, 'f', 2));

        if(fechaEmision.isValid()) {
            ui->dtFechaEmision->setDateTime(fechaEmision);
        }

        if(fechaVencimiento.isValid()) {
            ui->deFechaVencimiento->setDate(fechaVencimiento);
        }

        int indexEstado = ui->cbEstado->findData(estado);
        if(indexEstado >= 0) {
            ui->cbEstado->setCurrentIndex(indexEstado);
        }
    } else {
        qDebug() << "Error al cargar datos de la cuenta por cobrar:" << query.lastError().text();
    }
}

void Cuentascobrar::on_btnSalir_clicked()
{
    close();
}
