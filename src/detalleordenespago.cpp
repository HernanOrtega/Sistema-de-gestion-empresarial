#include "detalleordenespago.h"
#include "ui_detalleordenespago.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QDoubleValidator>

Detalleordenespago::Detalleordenespago(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Detalleordenespago),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo marrón moderno ------------------------------
    this->setStyleSheet("background-color: #f8f4f0;"); // Fondo marrón claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #a1887f; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtMonto->setStyleSheet(lineEditStyle);
    ui->txtSaldoAnterior->setStyleSheet(lineEditStyle);
    ui->txtSaldoActual->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #5d4037; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelOrdenPago->setStyleSheet(labelStyle);
    ui->labelCompra->setStyleSheet(labelStyle);
    ui->labelMonto->setStyleSheet(labelStyle);
    ui->labelSaldoAnterior->setStyleSheet(labelStyle);
    ui->labelSaldoActual->setStyleSheet(labelStyle);

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
    ui->cbOrdenPago->setStyleSheet(comboStyle);
    ui->cbCompra->setStyleSheet(comboStyle);

    ui->txtID->setEnabled(false);
    ui->txtSaldoActual->setEnabled(false); // Se calcula automáticamente

    // Configurar validación para montos (solo números)
    ui->txtMonto->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));
    ui->txtSaldoAnterior->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // Conectar señales para cálculo automático del saldo actual
    connect(ui->txtMonto, &QLineEdit::textChanged, this, [this](const QString &) {
        calcularSaldoActual();
    });
    connect(ui->txtSaldoAnterior, &QLineEdit::textChanged, this, [this](const QString &) {
        calcularSaldoActual();
    });

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

    ui->tablaDetalleOrdenesPago->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaDetalleOrdenesPago->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Detalleordenespago::filaSeleccionada);
}

Detalleordenespago::~Detalleordenespago()
{
    delete ui;
}

void Detalleordenespago::calcularSaldoActual()
{
    double monto = ui->txtMonto->text().toDouble();
    double saldoAnterior = ui->txtSaldoAnterior->text().toDouble();

    double saldoActual = saldoAnterior - monto;
    ui->txtSaldoActual->setText(QString::number(saldoActual, 'f', 2));
}

void Detalleordenespago::cargarCombos()
{
    // Cargar órdenes de pago activas
    ui->cbOrdenPago->clear();
    QSqlQuery query;

    QString ordenPagoQuery = "SELECT op.id, 'Orden Pago #' || op.id || ' - Proveedor: ' || COALESCE(p.razon_social, 'N/A') || ' - Total: ' || op.total || ' - Estado: ' || op.estado "
                             "FROM orden_de_pago op "
                             "LEFT JOIN proveedor p ON op.proveedor_id = p.id "
                             "WHERE op.estado != 'CANCELADA' AND op.estado != 'RECHAZADA' "
                             "ORDER BY op.fecha_orden DESC";

    if(query.exec(ordenPagoQuery)) {
        while(query.next()) {
            int id = query.value(0).toInt();
            QString descripcion = query.value(1).toString();
            ui->cbOrdenPago->addItem(descripcion, id);
        }
    } else {
        qDebug() << "Error cargando órdenes de pago:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar las órdenes de pago:\n%1").arg(query.lastError().text()));
    }

    // Cargar compras activas del mismo proveedor que la orden de pago seleccionada
    ui->cbCompra->clear();
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
}

void Detalleordenespago::cargarDatos()
{
    QString sql = "SELECT opd.id, "
                  "'Orden Pago #' || op.id || ' - Proveedor: ' || COALESCE(p1.razon_social, 'N/A') AS orden_pago, "
                  "'Compra #' || c.id || ' - Proveedor: ' || COALESCE(p2.razon_social, 'N/A') AS compra, "
                  "opd.monto, "
                  "opd.saldo_anterior, "
                  "opd.saldo_actual "
                  "FROM orden_de_pago_detalle opd "
                  "LEFT JOIN orden_de_pago op ON opd.orden_de_pago_id = op.id "
                  "LEFT JOIN compra c ON opd.compra_id = c.id "
                  "LEFT JOIN proveedor p1 ON op.proveedor_id = p1.id "
                  "LEFT JOIN proveedor p2 ON c.proveedor_id = p2.id "
                  "ORDER BY op.fecha_orden DESC, opd.id";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Orden Pago"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Compra"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Monto"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Saldo Anterior"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Saldo Actual"));

    ui->tablaDetalleOrdenesPago->setModel(modelo);
    ui->tablaDetalleOrdenesPago->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaDetalleOrdenesPago->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaDetalleOrdenesPago->horizontalHeader()->setStretchLastSection(true);
    ui->tablaDetalleOrdenesPago->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Detalleordenespago::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtMonto->clear();
    ui->txtSaldoAnterior->clear();
    ui->txtSaldoActual->clear();

    if(ui->cbOrdenPago->count() > 0)
        ui->cbOrdenPago->setCurrentIndex(0);
    if(ui->cbCompra->count() > 0)
        ui->cbCompra->setCurrentIndex(0);

    ui->cbOrdenPago->setFocus();
}

void Detalleordenespago::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Detalleordenespago::on_btnGuardar_clicked()
{
    int ordenPagoId = ui->cbOrdenPago->currentData().toInt();
    int compraId = ui->cbCompra->currentData().toInt();
    double monto = ui->txtMonto->text().toDouble();
    double saldoAnterior = ui->txtSaldoAnterior->text().toDouble();
    double saldoActual = ui->txtSaldoActual->text().toDouble();

    if(ordenPagoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una orden de pago."));
        return;
    }

    if(compraId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una compra."));
        return;
    }

    if(monto <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El monto debe ser mayor a cero."));
        return;
    }

    if(saldoAnterior < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El saldo anterior no puede ser negativo."));
        return;
    }

    if(saldoActual < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El saldo actual no puede ser negativo."));
        return;
    }

    if(monto > saldoAnterior) {
        QMessageBox::warning(this, tr("Atención"), tr("El monto no puede ser mayor al saldo anterior."));
        return;
    }

    // Verificar si ya existe un detalle para esta orden de pago y compra
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM orden_de_pago_detalle WHERE orden_de_pago_id = ? AND compra_id = ?");
    checkQuery.addBindValue(ordenPagoId);
    checkQuery.addBindValue(compraId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe un detalle para esta compra en la orden de pago seleccionada."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO orden_de_pago_detalle (orden_de_pago_id, compra_id, monto, saldo_anterior, saldo_actual) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(ordenPagoId);
    query.addBindValue(compraId);
    query.addBindValue(monto);
    query.addBindValue(saldoAnterior);
    query.addBindValue(saldoActual);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el detalle de la orden de pago:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Detalle de la orden de pago guardado correctamente."));
}

void Detalleordenespago::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int ordenPagoId = ui->cbOrdenPago->currentData().toInt();
    int compraId = ui->cbCompra->currentData().toInt();
    double monto = ui->txtMonto->text().toDouble();
    double saldoAnterior = ui->txtSaldoAnterior->text().toDouble();
    double saldoActual = ui->txtSaldoActual->text().toDouble();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle para modificar."));
        return;
    }

    if(ordenPagoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una orden de pago."));
        return;
    }

    if(compraId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una compra."));
        return;
    }

    if(monto <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El monto debe ser mayor a cero."));
        return;
    }

    if(saldoAnterior < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El saldo anterior no puede ser negativo."));
        return;
    }

    if(saldoActual < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El saldo actual no puede ser negativo."));
        return;
    }

    if(monto > saldoAnterior) {
        QMessageBox::warning(this, tr("Atención"), tr("El monto no puede ser mayor al saldo anterior."));
        return;
    }

    // Verificar si ya existe otro detalle para esta orden de pago y compra (excluyendo el actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM orden_de_pago_detalle WHERE orden_de_pago_id = ? AND compra_id = ? AND id != ?");
    checkQuery.addBindValue(ordenPagoId);
    checkQuery.addBindValue(compraId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otro detalle para esta compra en la orden de pago seleccionada."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE orden_de_pago_detalle SET orden_de_pago_id=?, compra_id=?, monto=?, saldo_anterior=?, saldo_actual=? WHERE id=?");
    query.addBindValue(ordenPagoId);
    query.addBindValue(compraId);
    query.addBindValue(monto);
    query.addBindValue(saldoAnterior);
    query.addBindValue(saldoActual);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el detalle de la orden de pago:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Detalle de la orden de pago modificado correctamente."));
}

void Detalleordenespago::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este detalle de la orden de pago?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM orden_de_pago_detalle WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el detalle de la orden de pago:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Detalle de la orden de pago eliminado correctamente."));
}

void Detalleordenespago::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
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
    query.prepare("SELECT orden_de_pago_id, compra_id, monto, saldo_anterior, saldo_actual "
                  "FROM orden_de_pago_detalle WHERE id = ?");
    query.addBindValue(idDetalle);

    if(query.exec() && query.next()) {
        int ordenPagoId = query.value(0).toInt();
        int compraId = query.value(1).toInt();
        double monto = query.value(2).toDouble();
        double saldoAnterior = query.value(3).toDouble();
        double saldoActual = query.value(4).toDouble();

        // Buscar por ID en lugar de por texto
        int indexOrdenPago = ui->cbOrdenPago->findData(ordenPagoId);
        if(indexOrdenPago >= 0) {
            ui->cbOrdenPago->setCurrentIndex(indexOrdenPago);
        }

        int indexCompra = ui->cbCompra->findData(compraId);
        if(indexCompra >= 0) {
            ui->cbCompra->setCurrentIndex(indexCompra);
        }

        ui->txtMonto->setText(QString::number(monto, 'f', 2));
        ui->txtSaldoAnterior->setText(QString::number(saldoAnterior, 'f', 2));
        ui->txtSaldoActual->setText(QString::number(saldoActual, 'f', 2));
    } else {
        qDebug() << "Error al cargar datos del detalle:" << query.lastError().text();
    }
}

void Detalleordenespago::on_btnSalir_clicked()
{
    close();
}
