#include "proveedores.h"
#include "ui_proveedores.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Proveedores::Proveedores(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Proveedores),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo moderno ------------------------------
    this->setStyleSheet("background-color: #f0f0f5;");

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border:1px solid #ccc; border-radius:4px; padding:4px;";
    ui->txtId->setStyleSheet(lineEditStyle);
    ui->txtRuc->setStyleSheet(lineEditStyle);
    ui->txtRazonSocial->setStyleSheet(lineEditStyle);
    ui->txtNombreComercial->setStyleSheet(lineEditStyle);
    ui->txtDireccion->setStyleSheet(lineEditStyle);
    ui->txtTelefono->setStyleSheet(lineEditStyle);
    ui->txtEmail->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #333333; font-weight: bold;";
    ui->labelId->setStyleSheet(labelStyle);
    ui->labelRuc->setStyleSheet(labelStyle);
    ui->labelRazonSocial->setStyleSheet(labelStyle);
    ui->labelNombreComercial->setStyleSheet(labelStyle);
    ui->labelDireccion->setStyleSheet(labelStyle);
    ui->labelTelefono->setStyleSheet(labelStyle);
    ui->labelEmail->setStyleSheet(labelStyle);
    ui->labelCiudad->setStyleSheet(labelStyle);
    ui->labelActivo->setStyleSheet(labelStyle);
    ui->labelFechaRegistro->setStyleSheet(labelStyle);

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

    ui->txtId->setEnabled(false);
    ui->dtFechaRegistro->setDisplayFormat("yyyy-MM-dd HH:mm:ss");

    // ------------------------------ Configuración tabla ------------------------------
    ui->tablaProveedores->setStyleSheet(R"(
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

    // Primero cargamos ciudades y datos
    cargarCiudades();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaProveedores->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Proveedores::filaSeleccionada);
}

Proveedores::~Proveedores()
{
    delete ui;
}

void Proveedores::cargarCiudades()
{
    ui->cbCiudad->clear();
    QSqlQuery query("SELECT id, nombre FROM ciudad ORDER BY nombre");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbCiudad->addItem(nombre, id);
    }
}

void Proveedores::cargarDatos()
{
    QString sql = R"(
        SELECT p.id,
               p.ruc,
               p.razon_social,
               p.nombre_comercial,
               p.direccion,
               p.telefono,
               p.email,
               c.nombre AS ciudad,
               CASE WHEN p.activo THEN 'Sí' ELSE 'No' END AS activo,
               p.fecha_registro
        FROM proveedor p
        LEFT JOIN ciudad c ON p.ciudad_id = c.id
        ORDER BY p.id
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("RUC"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Razón Social"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Nombre Comercial"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Dirección"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Teléfono"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Email"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Ciudad"));
    modelo->setHeaderData(8, Qt::Horizontal, tr("Activo"));
    modelo->setHeaderData(9, Qt::Horizontal, tr("Fecha Registro"));

    ui->tablaProveedores->setModel(modelo);
    ui->tablaProveedores->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaProveedores->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaProveedores->horizontalHeader()->setStretchLastSection(true);
    ui->tablaProveedores->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Proveedores::limpiarCampos()
{
    ui->txtId->clear();
    ui->txtRuc->clear();
    ui->txtRazonSocial->clear();
    ui->txtNombreComercial->clear();
    ui->txtDireccion->clear();
    ui->txtTelefono->clear();
    ui->txtEmail->clear();
    if(ui->cbCiudad->count() > 0)
        ui->cbCiudad->setCurrentIndex(0);
    ui->chkActivo->setChecked(true);
    ui->dtFechaRegistro->setDateTime(QDateTime::currentDateTime());
    ui->txtRuc->setFocus();
}

void Proveedores::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Proveedores::on_btnGuardar_clicked()
{
    QString ruc = ui->txtRuc->text().trimmed();
    QString razonSocial = ui->txtRazonSocial->text().trimmed();
    QString nombreCom = ui->txtNombreComercial->text().trimmed();
    QString direccion = ui->txtDireccion->text().trimmed();
    QString telefono = ui->txtTelefono->text().trimmed();
    QString email = ui->txtEmail->text().trimmed();
    int ciudadId = ui->cbCiudad->currentData().toInt();
    bool activo = ui->chkActivo->isChecked();
    QDateTime fecha = ui->dtFechaRegistro->dateTime();

    if(ruc.isEmpty() || razonSocial.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("RUC y Razón Social son obligatorios."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO proveedor (ruc, razon_social, nombre_comercial, direccion, telefono, email, ciudad_id, activo, fecha_registro) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(ruc);
    query.addBindValue(razonSocial);
    query.addBindValue(nombreCom);
    query.addBindValue(direccion);
    query.addBindValue(telefono);
    query.addBindValue(email);
    query.addBindValue(ciudadId);
    query.addBindValue(activo);
    query.addBindValue(fecha);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el proveedor:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Proveedor guardado correctamente."));
}

void Proveedores::on_btnModificar_clicked()
{
    QString id = ui->txtId->text().trimmed();
    QString ruc = ui->txtRuc->text().trimmed();
    QString razonSocial = ui->txtRazonSocial->text().trimmed();
    QString nombreCom = ui->txtNombreComercial->text().trimmed();
    QString direccion = ui->txtDireccion->text().trimmed();
    QString telefono = ui->txtTelefono->text().trimmed();
    QString email = ui->txtEmail->text().trimmed();
    int ciudadId = ui->cbCiudad->currentData().toInt();
    bool activo = ui->chkActivo->isChecked();
    QDateTime fecha = ui->dtFechaRegistro->dateTime();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un proveedor para modificar."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE proveedor SET ruc=?, razon_social=?, nombre_comercial=?, direccion=?, telefono=?, email=?, ciudad_id=?, activo=?, fecha_registro=? WHERE id=?");
    query.addBindValue(ruc);
    query.addBindValue(razonSocial);
    query.addBindValue(nombreCom);
    query.addBindValue(direccion);
    query.addBindValue(telefono);
    query.addBindValue(email);
    query.addBindValue(ciudadId);
    query.addBindValue(activo);
    query.addBindValue(fecha);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el proveedor:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Proveedor modificado correctamente."));
}

void Proveedores::on_btnEliminar_clicked()
{
    QString id = ui->txtId->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un proveedor para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este proveedor?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM proveedor WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el proveedor:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Proveedor eliminado correctamente."));
}

void Proveedores::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtId->setText(modelo->data(modelo->index(row,0)).toString());
    ui->txtRuc->setText(modelo->data(modelo->index(row,1)).toString());
    ui->txtRazonSocial->setText(modelo->data(modelo->index(row,2)).toString());
    ui->txtNombreComercial->setText(modelo->data(modelo->index(row,3)).toString());
    ui->txtDireccion->setText(modelo->data(modelo->index(row,4)).toString());
    ui->txtTelefono->setText(modelo->data(modelo->index(row,5)).toString());
    ui->txtEmail->setText(modelo->data(modelo->index(row,6)).toString());

    QString ciudadNombre = modelo->data(modelo->index(row,7)).toString();
    int indexCombo = ui->cbCiudad->findText(ciudadNombre);
    if(indexCombo >= 0) ui->cbCiudad->setCurrentIndex(indexCombo);

    ui->chkActivo->setChecked(modelo->data(modelo->index(row,8)).toString() == "Sí");

    QDateTime fecha = modelo->data(modelo->index(row,9)).toDateTime();
    ui->dtFechaRegistro->setDateTime(fecha);
}

void Proveedores::on_tablaProveedores_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);
}

void Proveedores::on_btnCerrar_clicked()
{
    close();
}
