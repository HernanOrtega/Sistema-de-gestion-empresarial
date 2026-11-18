#include "departamentos.h"
#include "ui_departamentos.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Departamentos::Departamentos(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Departamentos),
    modelo(new QSqlTableModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo verde moderno ------------------------------
    this->setStyleSheet("background-color: #f0fff5;"); // Fondo verde claro muy suave

    // Estilo para QLineEdit
    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #2ecc71; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtId->setStyleSheet(lineEditStyle);
    ui->txtNombre->setStyleSheet(lineEditStyle);

    // Estilo para QLabel
    QString labelStyle = "color: #27ae60; font-weight: bold; font-size: 14px;";
    ui->labelId->setStyleSheet(labelStyle);
    ui->labelNombre->setStyleSheet(labelStyle);

    // Estilo para botones en tono verde moderno
    QString btnStyle = R"(
        QPushButton {
            background-color: #2ecc71;
            color: #ffffff;
            font-weight: bold;
            border-radius: 8px;
            padding: 8px 16px;
            border: none;
            font-size: 14px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #27ae60;
        }
        QPushButton:pressed {
            background-color: #229954;
        }
        QPushButton:disabled {
            background-color: #d5f4e6;
            color: #a3e4c7;
        }
    )";

    ui->btnNuevo->setStyleSheet(btnStyle);
    ui->btnGuardar->setStyleSheet(btnStyle);
    ui->btnModificar->setStyleSheet(btnStyle);
    ui->btnEliminar->setStyleSheet(btnStyle);
    ui->btnCerrar->setStyleSheet(btnStyle);

    // ------------------------------ Configuración tabla en tonos verdes ------------------------------
    QString tablaStyle = R"(
        QTableView {
            background-color: #e8f8f1;
            alternate-background-color: #d5f4e6;
            gridline-color: #2ecc71;
            selection-background-color: #2ecc71;
            selection-color: white;
            border: 2px solid #2ecc71;
            border-radius: 8px;
            font-size: 13px;
        }
        QHeaderView::section {
            background-color: #27ae60;
            color: white;
            font-weight: bold;
            border: 1px solid #229954;
            padding: 8px;
            font-size: 14px;
        }
        QTableView::item {
            padding: 6px;
            border-bottom: 1px solid #a3e4c7;
            color: #145a32;
        }
        QTableView::item:selected {
            background-color: #2ecc71;
            color: white;
            font-weight: bold;
        }
        QTableView::item:hover {
            background-color: #d5f4e6;
        }
    )";

    ui->tablaDepartamentos->setStyleSheet(tablaStyle);

    // ------------------------------
    // Configuración de la tabla y modelo
    // ------------------------------
    ui->txtId->setEnabled(false);

    modelo->setTable("departamento");
    modelo->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelo->select();

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Nombre"));

    ui->tablaDepartamentos->setModel(modelo);
    ui->tablaDepartamentos->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaDepartamentos->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaDepartamentos->horizontalHeader()->setStretchLastSection(true);
    ui->tablaDepartamentos->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->tablaDepartamentos->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Departamentos::filaSeleccionada);

    cargarDatos();
}

Departamentos::~Departamentos()
{
    delete ui;
}

void Departamentos::cargarDatos()
{
    modelo->select();
    limpiarCampos();
}

void Departamentos::limpiarCampos()
{
    ui->txtId->clear();
    ui->txtNombre->clear();
    ui->txtNombre->setFocus();
}

void Departamentos::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Departamentos::on_btnGuardar_clicked()
{
    QString nombre = ui->txtNombre->text().trimmed();

    if (nombre.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Debes ingresar un nombre."));
        ui->txtNombre->setFocus();
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO departamento (nombre) VALUES (?)");
    query.addBindValue(nombre);

    if (!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"),
                              tr("No se pudo guardar el departamento:\n%1").arg(query.lastError().text()));
        qDebug() << "Error INSERT departamento:" << query.lastError().text();
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Departamento guardado correctamente."));
    limpiarCampos();
}

void Departamentos::on_btnModificar_clicked()
{
    QString id = ui->txtId->text().trimmed();
    QString nombre = ui->txtNombre->text().trimmed();

    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un registro para modificar."));
        return;
    }
    if (nombre.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("El nombre no puede quedar vacío."));
        ui->txtNombre->setFocus();
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE departamento SET nombre = ? WHERE id = ?");
    query.addBindValue(nombre);
    query.addBindValue(id.toInt());

    if (!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"),
                              tr("No se pudo modificar el departamento:\n%1").arg(query.lastError().text()));
        qDebug() << "Error UPDATE departamento:" << query.lastError().text();
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Departamento modificado correctamente."));
}

void Departamentos::on_btnEliminar_clicked()
{
    QString id = ui->txtId->text().trimmed();

    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un registro para eliminar."));
        return;
    }

    if (QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este departamento?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM departamento WHERE id = ?");
    query.addBindValue(id.toInt());

    if (!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"),
                              tr("No se pudo eliminar el departamento:\n%1").arg(query.lastError().text()));
        qDebug() << "Error DELETE departamento:" << query.lastError().text();
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Departamento eliminado correctamente."));
    limpiarCampos();
}

void Departamentos::on_tablaDepartamentos_clicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    QString id = modelo->data(modelo->index(index.row(), 0)).toString();
    QString nombre = modelo->data(modelo->index(index.row(), 1)).toString();

    ui->txtId->setText(id);
    ui->txtNombre->setText(nombre);
}

void Departamentos::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if (!current.isValid()) {
        ui->txtId->clear();
        ui->txtNombre->clear();
        return;
    }
    QString id = modelo->data(modelo->index(current.row(), 0)).toString();
    QString nombre = modelo->data(modelo->index(current.row(), 1)).toString();

    ui->txtId->setText(id);
    ui->txtNombre->setText(nombre);
}

void Departamentos::on_btnCerrar_clicked()
{
    close();
}
