#include "monedas.h"
#include "ui_monedas.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QStyledItemDelegate>

// ------------------------------
// Delegate para mostrar Sí/No en columna Activo
// ------------------------------
class BooleanDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QString displayText(const QVariant &value, const QLocale &locale) const override {
        Q_UNUSED(locale)
        bool b = value.toBool();
        return b ? "Sí" : "No";
    }
};

// ------------------------------
// Constructor
// ------------------------------
Monedas::Monedas(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Monedas),
    modelo(new QSqlTableModel(this))
{
    ui->setupUi(this);

    // ------------------------------
    // Estilo moderno
    // ------------------------------
    this->setStyleSheet("background-color: #f0f0f5;");

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border:1px solid #ccc; border-radius:4px; padding:4px;";
    ui->txtId->setStyleSheet(lineEditStyle);
    ui->txtCodigo->setStyleSheet(lineEditStyle);
    ui->txtNombre->setStyleSheet(lineEditStyle);
    ui->txtSimbolo->setStyleSheet(lineEditStyle);
    ui->txtCambio->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #333333; font-weight: bold;";
    ui->labelId->setStyleSheet(labelStyle);
    ui->labelCodigo->setStyleSheet(labelStyle);
    ui->labelNombre->setStyleSheet(labelStyle);
    ui->labelSimbolo->setStyleSheet(labelStyle);
    ui->labelCambio->setStyleSheet(labelStyle);
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

    // ------------------------------
    // Estilo de la tabla
    // ------------------------------
    ui->tablaMonedas->setStyleSheet(R"(
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
    // Configuración del modelo
    // ------------------------------
    ui->txtId->setEnabled(false);
    modelo->setTable("moneda");
    modelo->setEditStrategy(QSqlTableModel::OnManualSubmit);
    modelo->select();

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Código"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Nombre"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Símbolo"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Cambio"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Activo"));

    ui->tablaMonedas->setModel(modelo);
    ui->tablaMonedas->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaMonedas->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaMonedas->horizontalHeader()->setStretchLastSection(true);
    ui->tablaMonedas->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // ------------------------------
    // Aplicar delegate para columna Activo
    // ------------------------------
    ui->tablaMonedas->setItemDelegateForColumn(5, new BooleanDelegate(this));

    connect(ui->tablaMonedas->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Monedas::filaSeleccionada);

    cargarDatos();
}

// ------------------------------
// Destructor
// ------------------------------
Monedas::~Monedas()
{
    delete ui;
}

// ------------------------------
// Cargar datos
// ------------------------------
void Monedas::cargarDatos()
{
    modelo->select();
    limpiarCampos();
}

// ------------------------------
// Limpiar campos
// ------------------------------
void Monedas::limpiarCampos()
{
    ui->txtId->clear();
    ui->txtCodigo->clear();
    ui->txtNombre->clear();
    ui->txtSimbolo->clear();
    ui->txtCambio->clear();
    ui->chkActivo->setChecked(true);
    ui->txtCodigo->setFocus();
}

// ------------------------------
// Botones
// ------------------------------
void Monedas::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Monedas::on_btnGuardar_clicked()
{
    QString codigo = ui->txtCodigo->text().trimmed();
    QString nombre = ui->txtNombre->text().trimmed();
    QString simbolo = ui->txtSimbolo->text().trimmed();
    double cambio = ui->txtCambio->text().toDouble();
    bool activo = ui->chkActivo->isChecked();

    if(codigo.isEmpty() || nombre.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Código y nombre son obligatorios."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO moneda (codigo, nombre, simbolo, cambio, activo) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(codigo);
    query.addBindValue(nombre);
    query.addBindValue(simbolo);
    query.addBindValue(cambio);
    query.addBindValue(activo);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar la moneda:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Moneda guardada correctamente."));
}

void Monedas::on_btnModificar_clicked()
{
    QString id = ui->txtId->text().trimmed();
    QString codigo = ui->txtCodigo->text().trimmed();
    QString nombre = ui->txtNombre->text().trimmed();
    QString simbolo = ui->txtSimbolo->text().trimmed();
    double cambio = ui->txtCambio->text().toDouble();
    bool activo = ui->chkActivo->isChecked();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona una moneda para modificar."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE moneda SET codigo=?, nombre=?, simbolo=?, cambio=?, activo=? WHERE id=?");
    query.addBindValue(codigo);
    query.addBindValue(nombre);
    query.addBindValue(simbolo);
    query.addBindValue(cambio);
    query.addBindValue(activo);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar la moneda:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Moneda modificada correctamente."));
}

void Monedas::on_btnEliminar_clicked()
{
    QString id = ui->txtId->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona una moneda para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar esta moneda?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM moneda WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar la moneda:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Moneda eliminada correctamente."));
}

// ------------------------------
// Selección de fila
// ------------------------------
void Monedas::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtId->setText(modelo->data(modelo->index(row,0)).toString());
    ui->txtCodigo->setText(modelo->data(modelo->index(row,1)).toString());
    ui->txtNombre->setText(modelo->data(modelo->index(row,2)).toString());
    ui->txtSimbolo->setText(modelo->data(modelo->index(row,3)).toString());
    ui->txtCambio->setText(modelo->data(modelo->index(row,4)).toString());
    ui->chkActivo->setChecked(modelo->data(modelo->index(row,5)).toBool());
}

// ------------------------------
// Compatibilidad click en tabla
// ------------------------------
void Monedas::on_tablaMonedas_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);
}

// ------------------------------
// Cerrar ventana
// ------------------------------
void Monedas::on_btnCerrar_clicked()
{
    close();
}
