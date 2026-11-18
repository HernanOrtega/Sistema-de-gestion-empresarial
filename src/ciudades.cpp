#include "ciudades.h"
#include "ui_ciudades.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QSqlRelationalDelegate>
#include <QSqlRelation>
#include <QSqlRecord>

Ciudades::Ciudades(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Ciudades),
    modelo(new QSqlRelationalTableModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo verde moderno ------------------------------
    this->setStyleSheet("background-color: #f0fff5;"); // Fondo verde claro muy suave

    // Estilo para QLineEdit
    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #2ecc71; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtId->setStyleSheet(lineEditStyle);
    ui->txtNombre->setStyleSheet(lineEditStyle);
    ui->txtCodigoPostal->setStyleSheet(lineEditStyle);

    // Estilo para QLabel
    QString labelStyle = "color: #27ae60; font-weight: bold; font-size: 14px;";
    ui->labelNombre->setStyleSheet(labelStyle);
    ui->labelId->setStyleSheet(labelStyle);
    ui->labelCodigoPostal->setStyleSheet(labelStyle);
    ui->label_3->setStyleSheet(labelStyle);

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

    // Estilo para QComboBox
    QString comboStyle = "background-color: #ffffff; color: #000000; border: 1px solid #2ecc71; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->cbDepartamento->setStyleSheet(comboStyle);

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

    ui->tablaCiudades->setStyleSheet(tablaStyle);

    // ------------------------------
    // Configuración del modelo relacional
    // ------------------------------
    ui->txtId->setEnabled(false);

    modelo->setTable("ciudad");
    modelo->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Relación para mostrar el nombre del departamento en lugar del id
    modelo->setRelation(3, QSqlRelation("departamento", "id", "nombre")); // columna 3 = departamento_id

    modelo->select();

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Nombre"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Código Postal"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Departamento"));

    ui->tablaCiudades->setModel(modelo);
    ui->tablaCiudades->setItemDelegate(new QSqlRelationalDelegate(ui->tablaCiudades));
    ui->tablaCiudades->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaCiudades->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaCiudades->horizontalHeader()->setStretchLastSection(true);
    ui->tablaCiudades->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Conectar la señal de selección
    connect(ui->tablaCiudades->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Ciudades::filaSeleccionada);

    cargarDepartamentos();
    cargarDatos();
}

Ciudades::~Ciudades()
{
    delete ui;
}

void Ciudades::cargarDepartamentos()
{
    ui->cbDepartamento->clear();

    QSqlQuery query("SELECT id, nombre FROM departamento ORDER BY nombre");
    while (query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbDepartamento->addItem(nombre, id);
    }
}

void Ciudades::cargarDatos()
{
    modelo->select();
    limpiarCampos();
}

void Ciudades::limpiarCampos()
{
    ui->txtId->clear();
    ui->txtNombre->clear();
    ui->txtCodigoPostal->clear();
    if (ui->cbDepartamento->count() > 0)
        ui->cbDepartamento->setCurrentIndex(0);
    else
        ui->cbDepartamento->setCurrentIndex(-1);
    ui->txtNombre->setFocus();
}

void Ciudades::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Ciudades::on_btnGuardar_clicked()
{
    QString nombre = ui->txtNombre->text().trimmed();
    QString codigo = ui->txtCodigoPostal->text().trimmed();
    int deptId = ui->cbDepartamento->currentData().toInt();

    if (nombre.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Debes ingresar un nombre."));
        ui->txtNombre->setFocus();
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO ciudad (nombre, codigo_postal, departamento_id) VALUES (?, ?, ?)");
    query.addBindValue(nombre);
    query.addBindValue(codigo);
    query.addBindValue(deptId);

    if (!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"),
                              tr("No se pudo guardar la ciudad:\n%1").arg(query.lastError().text()));
        qDebug() << "Error INSERT ciudad:" << query.lastError().text();
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Ciudad guardada correctamente."));
    limpiarCampos();
}

void Ciudades::on_btnModificar_clicked()
{
    QString id = ui->txtId->text().trimmed();
    QString nombre = ui->txtNombre->text().trimmed();
    QString codigo = ui->txtCodigoPostal->text().trimmed();
    int deptId = ui->cbDepartamento->currentData().toInt();

    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un registro para modificar."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE ciudad SET nombre=?, codigo_postal=?, departamento_id=? WHERE id=?");
    query.addBindValue(nombre);
    query.addBindValue(codigo);
    query.addBindValue(deptId);
    query.addBindValue(id.toInt());

    if (!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"),
                              tr("No se pudo modificar la ciudad:\n%1").arg(query.lastError().text()));
        qDebug() << "Error UPDATE ciudad:" << query.lastError().text();
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Ciudad modificada correctamente."));
}

void Ciudades::on_btnEliminar_clicked()
{
    QString id = ui->txtId->text().trimmed();
    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un registro para eliminar."));
        return;
    }

    if (QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar esta ciudad?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM ciudad WHERE id=?");
    query.addBindValue(id.toInt());

    if (!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"),
                              tr("No se pudo eliminar la ciudad:\n%1").arg(query.lastError().text()));
        qDebug() << "Error DELETE ciudad:" << query.lastError().text();
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Ciudad eliminada correctamente."));
    limpiarCampos();
}

// -------------------------------------------------------------
// SOLUCIÓN CORREGIDA: Obtener el ID del departamento correctamente
// -------------------------------------------------------------
void Ciudades::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if (!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();

    // Obtener datos básicos directamente del modelo
    ui->txtId->setText(modelo->data(modelo->index(row, 0)).toString());
    ui->txtNombre->setText(modelo->data(modelo->index(row, 1)).toString());
    ui->txtCodigoPostal->setText(modelo->data(modelo->index(row, 2)).toString());

    // SOLUCIÓN: Obtener el departamento_id usando una consulta directa
    int ciudadId = modelo->data(modelo->index(row, 0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT departamento_id FROM ciudad WHERE id = ?");
    query.addBindValue(ciudadId);

    if (query.exec() && query.next()) {
        int departamentoId = query.value(0).toInt();

        // Buscar el índice correcto en el combobox
        int indexCombo = ui->cbDepartamento->findData(departamentoId);
        if (indexCombo >= 0) {
            ui->cbDepartamento->setCurrentIndex(indexCombo);
        } else {
            ui->cbDepartamento->setCurrentIndex(-1);
            qDebug() << "No se encontró el departamento ID:" << departamentoId;
        }
    } else {
        qDebug() << "Error al obtener departamento_id:" << query.lastError().text();
        ui->cbDepartamento->setCurrentIndex(-1);
    }
}

// -------------------------------------------------------------
// Función vacía para compatibilidad
// -------------------------------------------------------------
void Ciudades::on_tablaCiudades_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);
}

void Ciudades::on_btnCerrar_clicked()
{
    close();
}
