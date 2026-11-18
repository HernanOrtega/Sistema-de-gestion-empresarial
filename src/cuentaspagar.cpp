#include "cuentaspagar.h"
#include "ui_cuentaspagar.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QDoubleValidator>

Cuentaspagar::Cuentaspagar(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Cuentaspagar),
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
    ui->labelProveedor->setStyleSheet(labelStyle);
    ui->labelCompra->setStyleSheet(labelStyle);
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
    ui->cbProveedor->setStyleSheet(comboStyle);
    ui->cbCompra->setStyleSheet(comboStyle);
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

    ui->tablaCuentasPagar->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    if (ui->tablaCuentasPagar->selectionModel()) {
        connect(ui->tablaCuentasPagar->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                &Cuentaspagar::filaSeleccionada);
    }
}

Cuentaspagar::~Cuentaspagar()
{
    delete ui;
}

void Cuentaspagar::cargarCombos()
{
    // Cargar proveedores activos - CORREGIDO: usando razon_social
    ui->cbProveedor->clear();
    QSqlQuery query;
    if (!query.exec("SELECT id, razon_social FROM proveedor WHERE activo = true ORDER BY razon_social")) {
        qDebug() << "Error cargando proveedores:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar los proveedores:\n%1").arg(query.lastError().text()));
        return;
    }

    while(query.next()) {
        int id = query.value(0).toInt();
        QString razonSocial = query.value(1).toString();
        ui->cbProveedor->addItem(razonSocial, id);
    }

    // Cargar compras activas - CORREGIDO para las tablas reales
    ui->cbCompra->clear();

    // Consulta corregida usando las columnas reales
    QString compraQuery = "SELECT c.id, "
                          "'Compra #' || c.id || ' - Prov: ' || COALESCE(p.razon_social, 'N/A') || ' - Total: ' || c.total || ' - Fact: ' || COALESCE(c.nro_factura, 'S/F') "
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
}

void Cuentaspagar::cargarDatos()
{
    // CORREGIDO: Consulta usando las columnas reales de las tablas
    QString sql = "SELECT cp.id, "
                  "p.razon_social AS proveedor, "
                  "'Compra #' || c.id || ' - Fact: ' || COALESCE(c.nro_factura, 'S/F') AS compra, "
                  "cp.monto_total, "
                  "cp.saldo_pendiente, "
                  "TO_CHAR(cp.fecha_emision, 'YYYY-MM-DD HH24:MI') AS fecha_emision, "
                  "TO_CHAR(cp.fecha_vencimiento, 'YYYY-MM-DD') AS fecha_vencimiento, "
                  "cp.estado "
                  "FROM cuenta_a_pagar cp "
                  "LEFT JOIN proveedor p ON cp.proveedor_id = p.id "
                  "LEFT JOIN compra c ON cp.compra_id = c.id "
                  "ORDER BY cp.fecha_emision DESC";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Proveedor"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Compra"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Monto Total"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Saldo Pendiente"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Fecha Emisión"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Fecha Vencimiento"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Estado"));

    ui->tablaCuentasPagar->setModel(modelo);
    ui->tablaCuentasPagar->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaCuentasPagar->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaCuentasPagar->horizontalHeader()->setStretchLastSection(true);
    ui->tablaCuentasPagar->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Ajustar el ancho de las columnas
    ui->tablaCuentasPagar->setColumnWidth(0, 50);  // ID
    ui->tablaCuentasPagar->setColumnWidth(1, 200); // Proveedor
    ui->tablaCuentasPagar->setColumnWidth(2, 200); // Compra
    ui->tablaCuentasPagar->setColumnWidth(3, 100); // Monto Total
    ui->tablaCuentasPagar->setColumnWidth(4, 100); // Saldo Pendiente
    ui->tablaCuentasPagar->setColumnWidth(5, 150); // Fecha Emisión
    ui->tablaCuentasPagar->setColumnWidth(6, 120); // Fecha Vencimiento
    ui->tablaCuentasPagar->setColumnWidth(7, 100); // Estado

    limpiarCampos();
}

void Cuentaspagar::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtMontoTotal->clear();
    ui->txtSaldoPendiente->clear();

    if(ui->cbProveedor->count() > 0)
        ui->cbProveedor->setCurrentIndex(0);
    if(ui->cbCompra->count() > 0)
        ui->cbCompra->setCurrentIndex(0);
    if(ui->cbEstado->count() > 0)
        ui->cbEstado->setCurrentIndex(0); // PENDIENTE por defecto

    ui->dtFechaEmision->setDateTime(QDateTime::currentDateTime());
    ui->deFechaVencimiento->setDate(QDate::currentDate().addDays(30));

    ui->cbProveedor->setFocus();
}

void Cuentaspagar::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Cuentaspagar::on_btnGuardar_clicked()
{
    int proveedorId = ui->cbProveedor->currentData().toInt();
    int compraId = ui->cbCompra->currentData().toInt();
    double montoTotal = ui->txtMontoTotal->text().toDouble();
    double saldoPendiente = ui->txtSaldoPendiente->text().toDouble();
    QDateTime fechaEmision = ui->dtFechaEmision->dateTime();
    QDate fechaVencimiento = ui->deFechaVencimiento->date();
    QString estado = ui->cbEstado->currentData().toString();

    if(proveedorId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un proveedor."));
        return;
    }

    if(compraId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una compra."));
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

    // Verificar si ya existe una cuenta por pagar para esta compra
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM cuenta_a_pagar WHERE compra_id = ?");
    checkQuery.addBindValue(compraId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe una cuenta por pagar para esta compra."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO cuenta_a_pagar (proveedor_id, compra_id, monto_total, saldo_pendiente, fecha_emision, fecha_vencimiento, estado) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(proveedorId);
    query.addBindValue(compraId);
    query.addBindValue(montoTotal);
    query.addBindValue(saldoPendiente);
    query.addBindValue(fechaEmision);
    query.addBindValue(fechaVencimiento);
    query.addBindValue(estado);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar la cuenta por pagar:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Cuenta por pagar guardada correctamente."));
}

void Cuentaspagar::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int proveedorId = ui->cbProveedor->currentData().toInt();
    int compraId = ui->cbCompra->currentData().toInt();
    double montoTotal = ui->txtMontoTotal->text().toDouble();
    double saldoPendiente = ui->txtSaldoPendiente->text().toDouble();
    QDateTime fechaEmision = ui->dtFechaEmision->dateTime();
    QDate fechaVencimiento = ui->deFechaVencimiento->date();
    QString estado = ui->cbEstado->currentData().toString();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una cuenta por pagar para modificar."));
        return;
    }

    if(proveedorId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un proveedor."));
        return;
    }

    if(compraId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una compra."));
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

    // Verificar si ya existe otra cuenta por pagar para esta compra (excluyendo la actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM cuenta_a_pagar WHERE compra_id = ? AND id != ?");
    checkQuery.addBindValue(compraId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otra cuenta por pagar para esta compra."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE cuenta_a_pagar SET proveedor_id=?, compra_id=?, monto_total=?, saldo_pendiente=?, fecha_emision=?, fecha_vencimiento=?, estado=? WHERE id=?");
    query.addBindValue(proveedorId);
    query.addBindValue(compraId);
    query.addBindValue(montoTotal);
    query.addBindValue(saldoPendiente);
    query.addBindValue(fechaEmision);
    query.addBindValue(fechaVencimiento);
    query.addBindValue(estado);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar la cuenta por pagar:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Cuenta por pagar modificada correctamente."));
}

void Cuentaspagar::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una cuenta por pagar para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar esta cuenta por pagar?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM cuenta_a_pagar WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar la cuenta por pagar:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Cuenta por pagar eliminada correctamente."));
}

void Cuentaspagar::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
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
    query.prepare("SELECT proveedor_id, compra_id, monto_total, saldo_pendiente, fecha_emision, fecha_vencimiento, estado "
                  "FROM cuenta_a_pagar WHERE id = ?");
    query.addBindValue(idCuenta);

    if(query.exec() && query.next()) {
        int proveedorId = query.value(0).toInt();
        int compraId = query.value(1).toInt();
        double montoTotal = query.value(2).toDouble();
        double saldoPendiente = query.value(3).toDouble();
        QDateTime fechaEmision = query.value(4).toDateTime();
        QDate fechaVencimiento = query.value(5).toDate();
        QString estado = query.value(6).toString();

        // Buscar por ID en lugar de por texto
        int indexProveedor = ui->cbProveedor->findData(proveedorId);
        if(indexProveedor >= 0) {
            ui->cbProveedor->setCurrentIndex(indexProveedor);
        }

        int indexCompra = ui->cbCompra->findData(compraId);
        if(indexCompra >= 0) {
            ui->cbCompra->setCurrentIndex(indexCompra);
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
        qDebug() << "Error al cargar datos de la cuenta por pagar:" << query.lastError().text();
    }
}

void Cuentaspagar::on_btnSalir_clicked()
{
    close();
}
