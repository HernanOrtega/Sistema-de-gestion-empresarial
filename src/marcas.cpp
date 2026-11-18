#include "marcas.h"
#include "ui_marcas.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>

Marcas::Marcas(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Marcas),
    modelo(new QSqlTableModel(this))
{
    ui->setupUi(this);

    // ------------------------------
    // Estilo moderno
    // ------------------------------
    this->setStyleSheet("background-color: #f0f0f5;");

    QString lineEditStyle =
        "background-color: #ffffff; color: #000000; border:1px solid #ccc;"
        "border-radius:4px; padding:4px;";

    ui->txtId->setStyleSheet(lineEditStyle);
    ui->txtNombre->setStyleSheet(lineEditStyle);
    ui->txtDescripcion->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #333333; font-weight: bold;";
    ui->labelId->setStyleSheet(labelStyle);
    ui->labelNombre->setStyleSheet(labelStyle);
    ui->labelDescripcion->setStyleSheet(labelStyle);
    ui->labelActivo->setStyleSheet(labelStyle);

    QString btnStyle = R"(
        QPushButton {
            background-color: #007acc;
            color: white;
            font-weight: bold;
            border-radius: 6px;
            padding: 6px 12px;
            border: none;
        }
        QPushButton:hover {
            background-color: #005f99;
        }
        QPushButton:pressed {
            background-color: #004466;
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
    ui->btnCerrar->setStyleSheet(btnStyle);

    // Estilo tabla
    ui->tablaMarcas->setStyleSheet(R"(
        QTableView {
            background-color: #ffffff;
            alternate-background-color: #f9f9f9;
            gridline-color: #dddddd;
            selection-background-color: #007acc;
            selection-color: white;
            border: 1px solid #cccccc;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #e0e0e0;
            font-weight: bold;
            border: 1px solid #cccccc;
            padding: 6px;
        }
        QTableView::item {
            padding: 4px;
            border-bottom: 1px solid #eeeeee;
        }
        QTableView::item:selected {
            background-color: #007acc;
            color: white;
        }
    )");

    // ------------------------------
    // Configuración del modelo SQL
    // ------------------------------
    ui->txtId->setEnabled(false);

    modelo->setTable("marca");
    modelo->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelo->select();

    modelo->setHeaderData(0, Qt::Horizontal, "ID");
    modelo->setHeaderData(1, Qt::Horizontal, "Nombre");
    modelo->setHeaderData(2, Qt::Horizontal, "Descripción");
    modelo->setHeaderData(3, Qt::Horizontal, "Activo");

    ui->tablaMarcas->setModel(modelo);
    ui->tablaMarcas->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaMarcas->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaMarcas->horizontalHeader()->setStretchLastSection(true);
    ui->tablaMarcas->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Delegate para "Sí/No"
    ui->tablaMarcas->setItemDelegateForColumn(3, new BooleanDelegate(this));

    connect(ui->tablaMarcas->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Marcas::filaSeleccionada);

    cargarDatos();
}

Marcas::~Marcas()
{
    delete ui;
}

void Marcas::cargarDatos()
{
    modelo->select();
    limpiarCampos();
}

void Marcas::limpiarCampos()
{
    ui->txtId->clear();
    ui->txtNombre->clear();
    ui->txtDescripcion->clear();
    ui->chkActivo->setChecked(true);
    ui->txtNombre->setFocus();
}

void Marcas::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Marcas::on_btnGuardar_clicked()
{
    QString nombre = ui->txtNombre->text().trimmed();
    QString descripcion = ui->txtDescripcion->toPlainText().trimmed();
    bool activo = ui->chkActivo->isChecked();

    if (nombre.isEmpty()) {
        QMessageBox::warning(this, "Atención", "El nombre es obligatorio.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO marca (nombre, descripcion, activo) VALUES (?, ?, ?)");
    query.addBindValue(nombre);
    query.addBindValue(descripcion);
    query.addBindValue(activo);

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", query.lastError().text());
        return;
    }

    cargarDatos();
    QMessageBox::information(this, "Guardado", "Marca guardada correctamente.");
}

void Marcas::on_btnModificar_clicked()
{
    QString id = ui->txtId->text();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Atención", "Selecciona una marca.");
        return;
    }

    QString nombre = ui->txtNombre->text().trimmed();
    QString descripcion = ui->txtDescripcion->toPlainText().trimmed();
    bool activo = ui->chkActivo->isChecked();

    QSqlQuery query;
    query.prepare("UPDATE marca SET nombre=?, descripcion=?, activo=? WHERE id=?");
    query.addBindValue(nombre);
    query.addBindValue(descripcion);
    query.addBindValue(activo);
    query.addBindValue(id.toInt());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", query.lastError().text());
        return;
    }

    cargarDatos();
    QMessageBox::information(this, "Modificado", "Marca modificada correctamente.");
}

void Marcas::on_btnEliminar_clicked()
{
    QString id = ui->txtId->text();

    if (id.isEmpty()) {
        QMessageBox::warning(this, "Atención", "Selecciona una marca.");
        return;
    }

    if (QMessageBox::question(this, "Confirmar",
                              "¿Eliminar esta marca?") != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("DELETE FROM marca WHERE id=?");
    query.addBindValue(id.toInt());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", query.lastError().text());
        return;
    }

    cargarDatos();
    QMessageBox::information(this, "Eliminado", "Marca eliminada correctamente.");
}

void Marcas::filaSeleccionada(const QModelIndex &current, const QModelIndex &)
{
    if (!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();

    ui->txtId->setText(modelo->data(modelo->index(row, 0)).toString());
    ui->txtNombre->setText(modelo->data(modelo->index(row, 1)).toString());
    ui->txtDescripcion->setText(modelo->data(modelo->index(row, 2)).toString());
    ui->chkActivo->setChecked(modelo->data(modelo->index(row, 3)).toBool());
}

void Marcas::on_btnCerrar_clicked()
{
    close();
}

