#include "presupuestos.h"
#include "ui_presupuestos.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QDoubleValidator>

Presupuestos::Presupuestos(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Presupuestos),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo naranja moderno ------------------------------
    this->setStyleSheet("background-color: #fff8f0;"); // Fondo naranja claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb366; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtTotal->setStyleSheet(lineEditStyle);
    ui->txtObservaciones->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #cc5500; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelCliente->setStyleSheet(labelStyle);
    ui->labelFechaPresupuesto->setStyleSheet(labelStyle);
    ui->labelValidoHasta->setStyleSheet(labelStyle);
    ui->labelTotal->setStyleSheet(labelStyle);
    ui->labelEstado->setStyleSheet(labelStyle);
    ui->labelObservaciones->setStyleSheet(labelStyle);
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
    ui->cbCliente->setStyleSheet(comboStyle);
    ui->cbEstado->setStyleSheet(comboStyle);
    ui->cbUsuario->setStyleSheet(comboStyle);

    // Estilo para QDateTimeEdit y QDateEdit
    QString dateStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb366; border-radius: 4px; padding: 4px;";
    ui->dtFechaPresupuesto->setStyleSheet(dateStyle);
    ui->deValidoHasta->setStyleSheet(dateStyle);

    // Estilo para QTextEdit
    QString textEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffb366; border-radius: 4px; padding: 4px;";
    ui->txtObservaciones->setStyleSheet(textEditStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaPresupuesto->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaPresupuesto->setDateTime(QDateTime::currentDateTime());

    ui->deValidoHasta->setDisplayFormat("yyyy-MM-dd");
    ui->deValidoHasta->setDate(QDate::currentDate().addDays(30)); // 30 días por defecto

    // Configurar combo de estados
    ui->cbEstado->clear();
    ui->cbEstado->addItem("PENDIENTE", "PENDIENTE");
    ui->cbEstado->addItem("APROBADO", "APROBADO");
    ui->cbEstado->addItem("RECHAZADO", "RECHAZADO");
    ui->cbEstado->addItem("VENCIDO", "VENCIDO");

    // Configurar validación para montos (solo números)
    ui->txtTotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // ------------------------------ Configuración tabla en tonos naranjas ------------------------------
    ui->tablaPresupuestos->setStyleSheet(R"(
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
    connect(ui->tablaPresupuestos->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Presupuestos::filaSeleccionada);
}

Presupuestos::~Presupuestos()
{
    delete ui;
}

void Presupuestos::cargarCombos()
{
    // Cargar clientes activos
    ui->cbCliente->clear();
    QSqlQuery query("SELECT id, nombre_apellido_cliente FROM cliente WHERE activo = true ORDER BY nombre_apellido_cliente");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbCliente->addItem(nombre, id);
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

void Presupuestos::cargarDatos()
{
    QString sql = R"(
        SELECT p.id,
               c.nombre_apellido_cliente AS cliente,
               TO_CHAR(p.fecha_presupuesto, 'YYYY-MM-DD HH24:MI') AS fecha_presupuesto,
               TO_CHAR(p.valido_hasta, 'YYYY-MM-DD') AS valido_hasta,
               p.total,
               p.estado,
               p.observaciones,
               u.username AS usuario
        FROM presupuesto p
        LEFT JOIN cliente c ON p.cliente_id = c.id
        LEFT JOIN usuario u ON p.usuario_id = u.id
        ORDER BY p.fecha_presupuesto DESC
    )";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Cliente"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Fecha Presupuesto"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Válido Hasta"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Total"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Estado"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Observaciones"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Usuario"));

    ui->tablaPresupuestos->setModel(modelo);
    ui->tablaPresupuestos->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaPresupuestos->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaPresupuestos->horizontalHeader()->setStretchLastSection(true);
    ui->tablaPresupuestos->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Presupuestos::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtTotal->clear();
    ui->txtObservaciones->clear();

    if(ui->cbCliente->count() > 0)
        ui->cbCliente->setCurrentIndex(0);
    if(ui->cbEstado->count() > 0)
        ui->cbEstado->setCurrentIndex(0); // PENDIENTE por defecto
    if(ui->cbUsuario->count() > 0)
        ui->cbUsuario->setCurrentIndex(0);

    ui->dtFechaPresupuesto->setDateTime(QDateTime::currentDateTime());
    ui->deValidoHasta->setDate(QDate::currentDate().addDays(30));

    ui->cbCliente->setFocus();
}

void Presupuestos::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Presupuestos::on_btnGuardar_clicked()
{
    int clienteId = ui->cbCliente->currentData().toInt();
    QDateTime fechaPresupuesto = ui->dtFechaPresupuesto->dateTime();
    QDate validoHasta = ui->deValidoHasta->date();
    double total = ui->txtTotal->text().toDouble();
    QString estado = ui->cbEstado->currentData().toString();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(clienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cliente."));
        return;
    }

    if(total <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El total debe ser mayor a cero."));
        return;
    }

    if(validoHasta < QDate::currentDate()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha 'Válido Hasta' no puede ser anterior a la fecha actual."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO presupuesto (cliente_id, fecha_presupuesto, valido_hasta, total, estado, observaciones, usuario_id) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(clienteId);
    query.addBindValue(fechaPresupuesto);
    query.addBindValue(validoHasta);
    query.addBindValue(total);
    query.addBindValue(estado);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el presupuesto:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Presupuesto guardado correctamente."));
}

void Presupuestos::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int clienteId = ui->cbCliente->currentData().toInt();
    QDateTime fechaPresupuesto = ui->dtFechaPresupuesto->dateTime();
    QDate validoHasta = ui->deValidoHasta->date();
    double total = ui->txtTotal->text().toDouble();
    QString estado = ui->cbEstado->currentData().toString();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un presupuesto para modificar."));
        return;
    }

    if(clienteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cliente."));
        return;
    }

    if(total <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El total debe ser mayor a cero."));
        return;
    }

    if(validoHasta < QDate::currentDate()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha 'Válido Hasta' no puede ser anterior a la fecha actual."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE presupuesto SET cliente_id=?, fecha_presupuesto=?, valido_hasta=?, total=?, estado=?, observaciones=?, usuario_id=? WHERE id=?");
    query.addBindValue(clienteId);
    query.addBindValue(fechaPresupuesto);
    query.addBindValue(validoHasta);
    query.addBindValue(total);
    query.addBindValue(estado);
    query.addBindValue(observaciones);
    query.addBindValue(usuarioId);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el presupuesto:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Presupuesto modificado correctamente."));
}

void Presupuestos::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un presupuesto para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este presupuesto?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM presupuesto WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el presupuesto:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Presupuesto eliminado correctamente."));
}

void Presupuestos::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los IDs directamente de la base de datos para una búsqueda precisa
    int idPresupuesto = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT cliente_id, fecha_presupuesto, valido_hasta, total, estado, observaciones, usuario_id "
                  "FROM presupuesto WHERE id = ?");
    query.addBindValue(idPresupuesto);

    if(query.exec() && query.next()) {
        int clienteId = query.value(0).toInt();
        QDateTime fechaPresupuesto = query.value(1).toDateTime();
        QDate validoHasta = query.value(2).toDate();
        double total = query.value(3).toDouble();
        QString estado = query.value(4).toString();
        QString observaciones = query.value(5).toString();
        int usuarioId = query.value(6).toInt();

        // Buscar por ID en lugar de por texto
        int indexCliente = ui->cbCliente->findData(clienteId);
        if(indexCliente >= 0) {
            ui->cbCliente->setCurrentIndex(indexCliente);
        }

        if(fechaPresupuesto.isValid()) {
            ui->dtFechaPresupuesto->setDateTime(fechaPresupuesto);
        }

        if(validoHasta.isValid()) {
            ui->deValidoHasta->setDate(validoHasta);
        }

        ui->txtTotal->setText(QString::number(total, 'f', 2));

        int indexEstado = ui->cbEstado->findData(estado);
        if(indexEstado >= 0) {
            ui->cbEstado->setCurrentIndex(indexEstado);
        }

        ui->txtObservaciones->setPlainText(observaciones);

        int indexUsuario = ui->cbUsuario->findData(usuarioId);
        if(indexUsuario >= 0) {
            ui->cbUsuario->setCurrentIndex(indexUsuario);
        }
    } else {
        qDebug() << "Error al cargar datos del presupuesto:" << query.lastError().text();
    }
}

void Presupuestos::on_btnSalir_clicked()
{
    close();
}
