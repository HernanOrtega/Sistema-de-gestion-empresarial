#include "cobros.h"
#include "ui_cobros.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Cobros::Cobros(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Cobros),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo amarillo moderno ------------------------------
    this->setStyleSheet("background-color: #fff9e6;"); // Fondo amarillo claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffd966; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtMontoTotal->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #8b6b00; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelCaja->setStyleSheet(labelStyle);
    ui->labelCliente->setStyleSheet(labelStyle);
    ui->labelMontoTotal->setStyleSheet(labelStyle);
    ui->labelMoneda->setStyleSheet(labelStyle);
    ui->labelFechaCobro->setStyleSheet(labelStyle);
    ui->labelObservaciones->setStyleSheet(labelStyle);
    ui->labelUsuario->setStyleSheet(labelStyle);

    // Estilo para botones en tono amarillo/dorado
    QString btnStyle = R"(
        QPushButton {
            background-color: #e6b800;
            color: #ffffff;
            font-weight: bold;
            border-radius: 6px;
            padding: 6px 12px;
            border: none;
        }
        QPushButton:hover {
            background-color: #cc9900;
        }
        QPushButton:pressed {
            background-color: #b38600;
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
    QString comboStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffd966; border-radius: 4px; padding: 4px;";
    ui->cbCaja->setStyleSheet(comboStyle);
    ui->cbCliente->setStyleSheet(comboStyle);
    ui->cbMoneda->setStyleSheet(comboStyle);
    ui->cbUsuario->setStyleSheet(comboStyle);

    // Estilo para QPlainTextEdit
    ui->txtObservaciones->setStyleSheet("background-color: #ffffff; color: #000000; border: 1px solid #ffd966; border-radius: 4px; padding: 4px;");

    // Estilo para QDateTimeEdit
    QString dateTimeStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffd966; border-radius: 4px; padding: 4px;";
    ui->dtFechaCobro->setStyleSheet(dateTimeStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaCobro->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaCobro->setDateTime(QDateTime::currentDateTime());

    // Configurar validación para monto (solo números)
    ui->txtMontoTotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // ------------------------------ Configuración tabla en tonos amarillos ------------------------------
    ui->tablaCobros->setStyleSheet(R"(
        QTableView {
            background-color: #fffdf0;
            alternate-background-color: #fff5cc;
            gridline-color: #ffd966;
            selection-background-color: #e6b800;
            selection-color: white;
            border: 1px solid #ffd966;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #cc9900;
            color: white;
            font-weight: bold;
            border: 1px solid #b38600;
            padding: 6px;
        }
        QTableView::item {
            padding: 4px;
            border-bottom: 1px solid #ffe699;
            color: #333333;
        }
        QTableView::item:selected {
            background-color: #e6b800;
            color: white;
        }
    )");

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaCobros->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Cobros::filaSeleccionada);
}

Cobros::~Cobros()
{
    delete ui;
}

void Cobros::cargarCombos()
{
    // Cargar cajas activas
    ui->cbCaja->clear();
    QSqlQuery query("SELECT id, nombre FROM caja WHERE activa = true ORDER BY nombre");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbCaja->addItem(nombre, id);
    }

    // Cargar clientes activos
    ui->cbCliente->clear();
    query.exec("SELECT id, nombre_apellido_cliente FROM cliente WHERE activo = true ORDER BY nombre_apellido_cliente");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbCliente->addItem(nombre, id);
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

void Cobros::cargarDatos()
{
    QString sql = R"(
        SELECT c.id,
               ca.nombre AS caja,
               cl.nombre_apellido_cliente AS cliente,
               c.monto_total,
               m.nombre AS moneda,
               TO_CHAR(c.fecha_cobro, 'YYYY-MM-DD HH24:MI') AS fecha_cobro,
               c.observaciones,
               u.username AS usuario
        FROM cobro c
        LEFT JOIN caja ca ON c.caja_id = ca.id
        LEFT JOIN cliente cl ON c.cliente_id = cl.id
        LEFT JOIN moneda m ON c.moneda_id = m.id
        LEFT JOIN usuario u ON c.usuario_id = u.id
        ORDER BY c.fecha_cobro DESC
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Caja"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Cliente"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Monto Total"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Moneda"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Fecha Cobro"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Observaciones"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Usuario"));

    ui->tablaCobros->setModel(modelo);
    ui->tablaCobros->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaCobros->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaCobros->horizontalHeader()->setStretchLastSection(true);
    ui->tablaCobros->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Cobros::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtMontoTotal->clear();
    ui->txtObservaciones->clear();

    if(ui->cbCaja->count() > 0)
        ui->cbCaja->setCurrentIndex(0);
    if(ui->cbCliente->count() > 0)
        ui->cbCliente->setCurrentIndex(0);
    if(ui->cbMoneda->count() > 0)
        ui->cbMoneda->setCurrentIndex(0);
    if(ui->cbUsuario->count() > 0)
        ui->cbUsuario->setCurrentIndex(0);

    ui->dtFechaCobro->setDateTime(QDateTime::currentDateTime());
    ui->txtMontoTotal->setFocus();
}

void Cobros::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Cobros::on_btnGuardar_clicked()
{
    int cajaId = ui->cbCaja->currentData().toInt();
    int clienteId = ui->cbCliente->currentData().toInt();
    double montoTotal = ui->txtMontoTotal->text().toDouble();
    int monedaId = ui->cbMoneda->currentData().toInt();
    QDateTime fechaCobro = ui->dtFechaCobro->dateTime();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(cajaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una caja."));
        return;
    }

    if(clienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cliente."));
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
    query.prepare("INSERT INTO cobro (caja_id, cliente_id, monto_total, moneda_id, fecha_cobro, observaciones, usuario_id) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(cajaId);
    query.addBindValue(clienteId);
    query.addBindValue(montoTotal);
    query.addBindValue(monedaId);
    query.addBindValue(fechaCobro);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el cobro:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Cobro guardado correctamente."));
}

void Cobros::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int cajaId = ui->cbCaja->currentData().toInt();
    int clienteId = ui->cbCliente->currentData().toInt();
    double montoTotal = ui->txtMontoTotal->text().toDouble();
    int monedaId = ui->cbMoneda->currentData().toInt();
    QDateTime fechaCobro = ui->dtFechaCobro->dateTime();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cobro para modificar."));
        return;
    }

    if(cajaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una caja."));
        return;
    }

    if(clienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cliente."));
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
    query.prepare("UPDATE cobro SET caja_id=?, cliente_id=?, monto_total=?, moneda_id=?, fecha_cobro=?, observaciones=?, usuario_id=? WHERE id=?");
    query.addBindValue(cajaId);
    query.addBindValue(clienteId);
    query.addBindValue(montoTotal);
    query.addBindValue(monedaId);
    query.addBindValue(fechaCobro);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el cobro:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Cobro modificado correctamente."));
}

void Cobros::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cobro para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este cobro?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM cobro WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el cobro:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Cobro eliminado correctamente."));
}

void Cobros::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    QString cajaNombre = modelo->data(modelo->index(row,1)).toString();
    int indexCaja = ui->cbCaja->findText(cajaNombre);
    if(indexCaja >= 0) ui->cbCaja->setCurrentIndex(indexCaja);

    QString clienteNombre = modelo->data(modelo->index(row,2)).toString();
    int indexCliente = ui->cbCliente->findText(clienteNombre);
    if(indexCliente >= 0) ui->cbCliente->setCurrentIndex(indexCliente);

    ui->txtMontoTotal->setText(modelo->data(modelo->index(row,3)).toString());

    QString monedaNombre = modelo->data(modelo->index(row,4)).toString();
    int indexMoneda = ui->cbMoneda->findText(monedaNombre);
    if(indexMoneda >= 0) ui->cbMoneda->setCurrentIndex(indexMoneda);

    QString fechaStr = modelo->data(modelo->index(row,5)).toString();
    QDateTime fecha = QDateTime::fromString(fechaStr, "yyyy-MM-dd HH:mm");
    if(fecha.isValid()) ui->dtFechaCobro->setDateTime(fecha);

    ui->txtObservaciones->setPlainText(modelo->data(modelo->index(row,6)).toString());

    QString usuarioNombre = modelo->data(modelo->index(row,7)).toString();
    int indexUsuario = ui->cbUsuario->findText(usuarioNombre);
    if(indexUsuario >= 0) ui->cbUsuario->setCurrentIndex(indexUsuario);
}

void Cobros::on_btnSalir_clicked()
{
    close();
}
