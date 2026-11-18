#include "arqueos.h"
#include "ui_arqueos.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Arqueos::Arqueos(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Arqueos),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo amarillo moderno ------------------------------
    this->setStyleSheet("background-color: #fff9e6;"); // Fondo amarillo claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffd966; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtSaldoInicial->setStyleSheet(lineEditStyle);
    ui->txtSaldoFinal->setStyleSheet(lineEditStyle);
    ui->txtEstado->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #8b6b00; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelCaja->setStyleSheet(labelStyle);
    ui->labelUsuario->setStyleSheet(labelStyle);
    ui->labelFechaApertura->setStyleSheet(labelStyle);
    ui->labelFechaCierre->setStyleSheet(labelStyle);
    ui->labelSaldoInicial->setStyleSheet(labelStyle);
    ui->labelSaldoFinal->setStyleSheet(labelStyle);
    ui->labelEstado->setStyleSheet(labelStyle);

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
    ui->btnCerrarArqueo->setStyleSheet(btnStyle);
    ui->btnSalir->setStyleSheet(btnStyle);

    // Estilo especial para botón Cerrar Arqueo (verde cuando está activo)
    QString btnCerrarStyle = R"(
        QPushButton {
            background-color: #28a745;
            color: #ffffff;
            font-weight: bold;
            border-radius: 6px;
            padding: 6px 12px;
            border: none;
        }
        QPushButton:hover {
            background-color: #218838;
        }
        QPushButton:pressed {
            background-color: #1e7e34;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
        }
    )";
    ui->btnCerrarArqueo->setStyleSheet(btnCerrarStyle);

    // Estilo para QComboBox
    QString comboStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffd966; border-radius: 4px; padding: 4px;";
    ui->cbCaja->setStyleSheet(comboStyle);
    ui->cbUsuario->setStyleSheet(comboStyle);

    // Estilo para QDateTimeEdit
    QString dateTimeStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffd966; border-radius: 4px; padding: 4px;";
    ui->dtFechaApertura->setStyleSheet(dateTimeStyle);
    ui->dtFechaCierre->setStyleSheet(dateTimeStyle);

    ui->txtID->setEnabled(false);
    ui->txtEstado->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaApertura->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaCierre->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaApertura->setDateTime(QDateTime::currentDateTime());
    ui->dtFechaCierre->setDateTime(QDateTime::currentDateTime());

    // Configurar validación para saldos (solo números)
    ui->txtSaldoInicial->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));
    ui->txtSaldoFinal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // ------------------------------ Configuración tabla en tonos amarillos ------------------------------
    ui->tablaArqueos->setStyleSheet(R"(
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
    connect(ui->tablaArqueos->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Arqueos::filaSeleccionada);
}

Arqueos::~Arqueos()
{
    delete ui;
}

void Arqueos::cargarCombos()
{
    // Cargar cajas activas
    ui->cbCaja->clear();
    QSqlQuery query("SELECT id, nombre FROM caja WHERE activa = true ORDER BY nombre");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbCaja->addItem(nombre, id);
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

void Arqueos::cargarDatos()
{
    QString sql = R"(
        SELECT a.id,
               c.nombre AS caja,
               u.username AS usuario,
               TO_CHAR(a.fecha_apertura, 'YYYY-MM-DD HH24:MI') AS fecha_apertura,
               TO_CHAR(a.fecha_cierre, 'YYYY-MM-DD HH24:MI') AS fecha_cierre,
               a.saldo_inicial,
               a.saldo_final,
               a.estado
        FROM arqueo a
        LEFT JOIN caja c ON a.caja_id = c.id
        LEFT JOIN usuario u ON a.usuario_id = u.id
        ORDER BY a.fecha_apertura DESC
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Caja"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Usuario"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Fecha Apertura"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Fecha Cierre"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Saldo Inicial"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Saldo Final"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Estado"));

    ui->tablaArqueos->setModel(modelo);
    ui->tablaArqueos->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaArqueos->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaArqueos->horizontalHeader()->setStretchLastSection(true);
    ui->tablaArqueos->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Arqueos::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtSaldoInicial->clear();
    ui->txtSaldoFinal->clear();
    ui->txtEstado->clear();

    if(ui->cbCaja->count() > 0)
        ui->cbCaja->setCurrentIndex(0);
    if(ui->cbUsuario->count() > 0)
        ui->cbUsuario->setCurrentIndex(0);

    ui->dtFechaApertura->setDateTime(QDateTime::currentDateTime());
    ui->dtFechaCierre->setDateTime(QDateTime::currentDateTime());

    ui->txtSaldoInicial->setFocus();
    actualizarEstadoBotones("");
}

void Arqueos::actualizarEstadoBotones(const QString &estado)
{
    bool esAbierto = (estado == "ABIERTO");

    ui->btnModificar->setEnabled(esAbierto);
    ui->btnEliminar->setEnabled(esAbierto);
    ui->btnCerrarArqueo->setEnabled(esAbierto);
    ui->txtSaldoFinal->setEnabled(esAbierto);

    // Cambiar texto del botón según estado
    if (esAbierto) {
        ui->btnCerrarArqueo->setText("Cerrar Arqueo");
    } else {
        ui->btnCerrarArqueo->setText("Arqueo Cerrado");
    }
}

void Arqueos::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Arqueos::on_btnGuardar_clicked()
{
    int cajaId = ui->cbCaja->currentData().toInt();
    int usuarioId = ui->cbUsuario->currentData().toInt();
    QDateTime fechaApertura = ui->dtFechaApertura->dateTime();
    double saldoInicial = ui->txtSaldoInicial->text().toDouble();

    if(cajaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una caja."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    if(saldoInicial < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El saldo inicial no puede ser negativo."));
        return;
    }

    // Verificar si ya existe un arqueo abierto para esta caja
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM arqueo WHERE caja_id = ? AND estado = 'ABIERTO'");
    checkQuery.addBindValue(cajaId);
    if(checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe un arqueo abierto para esta caja. Cierre el arqueo actual antes de abrir uno nuevo."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO arqueo (caja_id, usuario_id, fecha_apertura, saldo_inicial, estado) "
                  "VALUES (?, ?, ?, ?, 'ABIERTO')");
    query.addBindValue(cajaId);
    query.addBindValue(usuarioId);
    query.addBindValue(fechaApertura);
    query.addBindValue(saldoInicial);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el arqueo:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Arqueo guardado correctamente."));
}

void Arqueos::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int cajaId = ui->cbCaja->currentData().toInt();
    int usuarioId = ui->cbUsuario->currentData().toInt();
    QDateTime fechaApertura = ui->dtFechaApertura->dateTime();
    double saldoInicial = ui->txtSaldoInicial->text().toDouble();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un arqueo para modificar."));
        return;
    }

    if(cajaId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una caja."));
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        return;
    }

    if(saldoInicial < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El saldo inicial no puede ser negativo."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE arqueo SET caja_id=?, usuario_id=?, fecha_apertura=?, saldo_inicial=? WHERE id=? AND estado = 'ABIERTO'");
    query.addBindValue(cajaId);
    query.addBindValue(usuarioId);
    query.addBindValue(fechaApertura);
    query.addBindValue(saldoInicial);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el arqueo:\n%1").arg(query.lastError().text()));
        return;
    }

    if(query.numRowsAffected() == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("No se puede modificar un arqueo cerrado."));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Arqueo modificado correctamente."));
}

void Arqueos::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un arqueo para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este arqueo?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM arqueo WHERE id=? AND estado = 'ABIERTO'");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el arqueo:\n%1").arg(query.lastError().text()));
        return;
    }

    if(query.numRowsAffected() == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("No se puede eliminar un arqueo cerrado."));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Arqueo eliminado correctamente."));
}

void Arqueos::on_btnCerrarArqueo_clicked()
{
    QString id = ui->txtID->text().trimmed();
    double saldoFinal = ui->txtSaldoFinal->text().toDouble();
    QDateTime fechaCierre = ui->dtFechaCierre->dateTime();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un arqueo para cerrar."));
        return;
    }

    if(saldoFinal < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El saldo final no puede ser negativo."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar cierre"),
                              tr("¿Estás seguro que deseas cerrar este arqueo?\nEsta acción no se puede deshacer."),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("UPDATE arqueo SET fecha_cierre=?, saldo_final=?, estado='CERRADO' WHERE id=? AND estado = 'ABIERTO'");
    query.addBindValue(fechaCierre);
    query.addBindValue(saldoFinal);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al cerrar"), tr("No se pudo cerrar el arqueo:\n%1").arg(query.lastError().text()));
        return;
    }

    if(query.numRowsAffected() == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El arqueo ya está cerrado o no existe."));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Cerrado"), tr("Arqueo cerrado correctamente."));
}

void Arqueos::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
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

    QString usuarioNombre = modelo->data(modelo->index(row,2)).toString();
    int indexUsuario = ui->cbUsuario->findText(usuarioNombre);
    if(indexUsuario >= 0) ui->cbUsuario->setCurrentIndex(indexUsuario);

    QString fechaAperturaStr = modelo->data(modelo->index(row,3)).toString();
    QDateTime fechaApertura = QDateTime::fromString(fechaAperturaStr, "yyyy-MM-dd HH:mm");
    if(fechaApertura.isValid()) ui->dtFechaApertura->setDateTime(fechaApertura);

    QString fechaCierreStr = modelo->data(modelo->index(row,4)).toString();
    QDateTime fechaCierre = QDateTime::fromString(fechaCierreStr, "yyyy-MM-dd HH:mm");
    if(fechaCierre.isValid()) {
        ui->dtFechaCierre->setDateTime(fechaCierre);
    } else {
        ui->dtFechaCierre->setDateTime(QDateTime::currentDateTime());
    }

    ui->txtSaldoInicial->setText(modelo->data(modelo->index(row,5)).toString());
    ui->txtSaldoFinal->setText(modelo->data(modelo->index(row,6)).toString());

    QString estado = modelo->data(modelo->index(row,7)).toString();
    ui->txtEstado->setText(estado);

    actualizarEstadoBotones(estado);
}

void Arqueos::on_btnSalir_clicked()
{
    close();
}
