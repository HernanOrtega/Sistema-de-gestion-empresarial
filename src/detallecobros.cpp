#include "detallecobros.h"
#include "ui_detallecobros.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Detallecobros::Detallecobros(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Detallecobros),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo amarillo moderno ------------------------------
    this->setStyleSheet("background-color: #fff9e6;"); // Fondo amarillo claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffd966; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtMonto->setStyleSheet(lineEditStyle);
    ui->txtReferencia->setStyleSheet(lineEditStyle);
    ui->txtBanco->setStyleSheet(lineEditStyle);
    ui->txtNroCheque->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #8b6b00; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelCobro->setStyleSheet(labelStyle);
    ui->labelCobroTipo->setStyleSheet(labelStyle);
    ui->labelMonto->setStyleSheet(labelStyle);
    ui->labelReferencia->setStyleSheet(labelStyle);
    ui->labelBanco->setStyleSheet(labelStyle);
    ui->labelNroCheque->setStyleSheet(labelStyle);

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
    ui->cbCobro->setStyleSheet(comboStyle);
    ui->cbCobroTipo->setStyleSheet(comboStyle);

    ui->txtID->setEnabled(false);

    // Configurar validación para monto (solo números)
    ui->txtMonto->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // ------------------------------ Configuración tabla en tonos amarillos ------------------------------
    ui->tablaDetalleCobros->setStyleSheet(R"(
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
    connect(ui->tablaDetalleCobros->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Detallecobros::filaSeleccionada);
}

Detallecobros::~Detallecobros()
{
    delete ui;
}

void Detallecobros::cargarCombos()
{
    // Cargar cobros
    ui->cbCobro->clear();
    QSqlQuery query(R"(
        SELECT c.id,
               'Cobro #' || c.id || ' - ' || cl.nombre_apellido_cliente || ' - ' || c.monto_total || ' ' || m.nombre
        FROM cobro c
        LEFT JOIN cliente cl ON c.cliente_id = cl.id
        LEFT JOIN moneda m ON c.moneda_id = m.id
        ORDER BY c.id DESC
    )");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString descripcion = query.value(1).toString();
        ui->cbCobro->addItem(descripcion, id);
    }

    // Cargar tipos de cobro
    ui->cbCobroTipo->clear();
    query.exec("SELECT id, nombre FROM cobro_tipo ORDER BY nombre");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbCobroTipo->addItem(nombre, id);
    }
}

void Detallecobros::cargarDatos()
{
    QString sql = R"(
        SELECT cd.id,
               'Cobro #' || c.id || ' - ' || cl.nombre_apellido_cliente AS cobro,
               ct.nombre AS tipo_cobro,
               cd.monto,
               cd.referencia,
               cd.banco,
               cd.nro_cheque
        FROM cobro_detalle cd
        LEFT JOIN cobro c ON cd.cobro_id = c.id
        LEFT JOIN cliente cl ON c.cliente_id = cl.id
        LEFT JOIN cobro_tipo ct ON cd.cobro_tipo_id = ct.id
        ORDER BY cd.id DESC
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Cobro"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Tipo Cobro"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Monto"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Referencia"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Banco"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Nro Cheque"));

    ui->tablaDetalleCobros->setModel(modelo);
    ui->tablaDetalleCobros->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaDetalleCobros->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaDetalleCobros->horizontalHeader()->setStretchLastSection(true);
    ui->tablaDetalleCobros->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Detallecobros::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtMonto->clear();
    ui->txtReferencia->clear();
    ui->txtBanco->clear();
    ui->txtNroCheque->clear();

    if(ui->cbCobro->count() > 0)
        ui->cbCobro->setCurrentIndex(0);
    if(ui->cbCobroTipo->count() > 0)
        ui->cbCobroTipo->setCurrentIndex(0);

    ui->txtMonto->setFocus();
}

void Detallecobros::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Detallecobros::on_btnGuardar_clicked()
{
    int cobroId = ui->cbCobro->currentData().toInt();
    int cobroTipoId = ui->cbCobroTipo->currentData().toInt();
    double monto = ui->txtMonto->text().toDouble();
    QString referencia = ui->txtReferencia->text().trimmed();
    QString banco = ui->txtBanco->text().trimmed();
    QString nroCheque = ui->txtNroCheque->text().trimmed();

    if(cobroId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cobro."));
        return;
    }

    if(cobroTipoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un tipo de cobro."));
        return;
    }

    if(monto <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El monto debe ser mayor a cero."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO cobro_detalle (cobro_id, cobro_tipo_id, monto, referencia, banco, nro_cheque) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(cobroId);
    query.addBindValue(cobroTipoId);
    query.addBindValue(monto);
    query.addBindValue(referencia);
    query.addBindValue(banco);
    query.addBindValue(nroCheque);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el detalle de cobro:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Detalle de cobro guardado correctamente."));
}

void Detallecobros::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int cobroId = ui->cbCobro->currentData().toInt();
    int cobroTipoId = ui->cbCobroTipo->currentData().toInt();
    double monto = ui->txtMonto->text().toDouble();
    QString referencia = ui->txtReferencia->text().trimmed();
    QString banco = ui->txtBanco->text().trimmed();
    QString nroCheque = ui->txtNroCheque->text().trimmed();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle de cobro para modificar."));
        return;
    }

    if(cobroId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un cobro."));
        return;
    }

    if(cobroTipoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un tipo de cobro."));
        return;
    }

    if(monto <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El monto debe ser mayor a cero."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE cobro_detalle SET cobro_id=?, cobro_tipo_id=?, monto=?, referencia=?, banco=?, nro_cheque=? WHERE id=?");
    query.addBindValue(cobroId);
    query.addBindValue(cobroTipoId);
    query.addBindValue(monto);
    query.addBindValue(referencia);
    query.addBindValue(banco);
    query.addBindValue(nroCheque);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el detalle de cobro:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Detalle de cobro modificado correctamente."));
}

void Detallecobros::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle de cobro para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este detalle de cobro?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM cobro_detalle WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el detalle de cobro:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Detalle de cobro eliminado correctamente."));
}

void Detallecobros::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    QString cobroDesc = modelo->data(modelo->index(row,1)).toString();
    int indexCobro = ui->cbCobro->findText(cobroDesc);
    if(indexCobro >= 0) ui->cbCobro->setCurrentIndex(indexCobro);

    QString tipoCobro = modelo->data(modelo->index(row,2)).toString();
    int indexTipo = ui->cbCobroTipo->findText(tipoCobro);
    if(indexTipo >= 0) ui->cbCobroTipo->setCurrentIndex(indexTipo);

    ui->txtMonto->setText(modelo->data(modelo->index(row,3)).toString());
    ui->txtReferencia->setText(modelo->data(modelo->index(row,4)).toString());
    ui->txtBanco->setText(modelo->data(modelo->index(row,5)).toString());
    ui->txtNroCheque->setText(modelo->data(modelo->index(row,6)).toString());
}

void Detallecobros::on_btnSalir_clicked()
{
    close();
}
