#include "caja.h"
#include "ui_caja.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Caja::Caja(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Caja),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo amarillo moderno ------------------------------
    this->setStyleSheet("background-color: #fff9e6;"); // Fondo amarillo claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #ffd966; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtNombre->setStyleSheet(lineEditStyle);
    ui->txtSaldoInicial->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #8b6b00; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelNombre->setStyleSheet(labelStyle);
    ui->labelDescripcion->setStyleSheet(labelStyle);
    ui->labelSaldoInicial->setStyleSheet(labelStyle);
    ui->labelActiva->setStyleSheet(labelStyle);

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

    // Estilo para QPlainTextEdit
    ui->txtDescripcion->setStyleSheet("background-color: #ffffff; color: #000000; border: 1px solid #ffd966; border-radius: 4px; padding: 4px;");

    // Estilo para QCheckBox
    ui->chkActiva->setStyleSheet("color: #8b6b00; font-weight: bold;");

    ui->txtID->setEnabled(false);

    // Configurar validación para saldo (solo números)
    ui->txtSaldoInicial->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

    // ------------------------------ Configuración tabla en tonos amarillos ------------------------------
    ui->tablaCajas->setStyleSheet(R"(
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

    // Cargar datos
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaCajas->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Caja::filaSeleccionada);
}

Caja::~Caja()
{
    delete ui;
}

void Caja::cargarDatos()
{
    QString sql = R"(
        SELECT id,
               nombre,
               descripcion,
               saldo_inicial,
               CASE WHEN activa THEN 'Sí' ELSE 'No' END AS activa
        FROM caja
        ORDER BY id
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Nombre"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Descripción"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Saldo Inicial"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Activa"));

    ui->tablaCajas->setModel(modelo);
    ui->tablaCajas->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaCajas->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaCajas->horizontalHeader()->setStretchLastSection(true);
    ui->tablaCajas->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Caja::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtNombre->clear();
    ui->txtDescripcion->clear();
    ui->txtSaldoInicial->clear();
    ui->chkActiva->setChecked(true);
    ui->txtNombre->setFocus();
}

void Caja::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Caja::on_btnGuardar_clicked()
{
    QString nombre = ui->txtNombre->text().trimmed();
    QString descripcion = ui->txtDescripcion->toPlainText().trimmed();
    double saldoInicial = ui->txtSaldoInicial->text().toDouble();
    bool activa = ui->chkActiva->isChecked();

    if(nombre.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("El nombre de la caja es obligatorio."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO caja (nombre, descripcion, saldo_inicial, activa) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(nombre);
    query.addBindValue(descripcion);
    query.addBindValue(saldoInicial);
    query.addBindValue(activa);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar la caja:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Caja guardada correctamente."));
}

void Caja::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    QString nombre = ui->txtNombre->text().trimmed();
    QString descripcion = ui->txtDescripcion->toPlainText().trimmed();
    double saldoInicial = ui->txtSaldoInicial->text().toDouble();
    bool activa = ui->chkActiva->isChecked();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona una caja para modificar."));
        return;
    }

    if(nombre.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("El nombre de la caja es obligatorio."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE caja SET nombre=?, descripcion=?, saldo_inicial=?, activa=? WHERE id=?");
    query.addBindValue(nombre);
    query.addBindValue(descripcion);
    query.addBindValue(saldoInicial);
    query.addBindValue(activa);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar la caja:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Caja modificada correctamente."));
}

void Caja::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona una caja para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar esta caja?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM caja WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar la caja:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Caja eliminada correctamente."));
}

void Caja::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());
    ui->txtNombre->setText(modelo->data(modelo->index(row,1)).toString());
    ui->txtDescripcion->setPlainText(modelo->data(modelo->index(row,2)).toString());
    ui->txtSaldoInicial->setText(modelo->data(modelo->index(row,3)).toString());
    ui->chkActiva->setChecked(modelo->data(modelo->index(row,4)).toString() == "Sí");
}

void Caja::on_btnSalir_clicked()
{
    close();
}
