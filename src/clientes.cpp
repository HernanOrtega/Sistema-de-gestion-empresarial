#include "clientes.h"
#include "ui_clientes.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Clientes::Clientes(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Clientes),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo moderno ------------------------------
    this->setStyleSheet("background-color: #f0f0f5;");

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border:1px solid #ccc; border-radius:4px; padding:4px;";
    ui->txtId->setStyleSheet(lineEditStyle);
    ui->txtRucCi->setStyleSheet(lineEditStyle);
    ui->txtNombreApellido->setStyleSheet(lineEditStyle);
    ui->txtDireccion->setStyleSheet(lineEditStyle);
    ui->txtTelefono->setStyleSheet(lineEditStyle);
    ui->txtEmail->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #333333; font-weight: bold;";
    ui->labelId->setStyleSheet(labelStyle);
    ui->labelRucCi->setStyleSheet(labelStyle);
    ui->labelNombreApellido->setStyleSheet(labelStyle);
    ui->labelDireccion->setStyleSheet(labelStyle);
    ui->labelTelefono->setStyleSheet(labelStyle);
    ui->labelEmail->setStyleSheet(labelStyle);
    ui->labelCiudad->setStyleSheet(labelStyle);
    ui->labelActivo->setStyleSheet(labelStyle);
    ui->labelFechaNacimiento->setStyleSheet(labelStyle);

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
    ui->dtFechaNacimiento->setDisplayFormat("yyyy-MM-dd");

    // ------------------------------ Configuración tabla ------------------------------
    ui->tablaClientes->setStyleSheet(R"(
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
    connect(ui->tablaClientes->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Clientes::filaSeleccionada);
}

Clientes::~Clientes()
{
    delete ui;
}

void Clientes::cargarCiudades()
{
    ui->cbCiudad->clear();
    QSqlQuery query("SELECT id, nombre FROM ciudad ORDER BY nombre");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbCiudad->addItem(nombre, id);
    }
}

void Clientes::cargarDatos()
{
    QString sql = R"(
        SELECT cl.id,
               cl.ruc_ci,
               cl.nombre_apellido_cliente,
               cl.direccion,
               cl.telefono,
               cl.email,
               c.nombre AS ciudad,
               CASE WHEN cl.activo THEN 'Sí' ELSE 'No' END AS activo,
               cl.fecha_nacimiento
        FROM cliente cl
        LEFT JOIN ciudad c ON cl.ciudad_id = c.id
        ORDER BY cl.id
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("RUC/CI"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Nombre y Apellido"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Dirección"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Teléfono"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Email"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Ciudad"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Activo"));
    modelo->setHeaderData(8, Qt::Horizontal, tr("Fecha Nacimiento"));

    ui->tablaClientes->setModel(modelo);
    ui->tablaClientes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaClientes->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaClientes->horizontalHeader()->setStretchLastSection(true);
    ui->tablaClientes->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Clientes::limpiarCampos()
{
    ui->txtId->clear();
    ui->txtRucCi->clear();
    ui->txtNombreApellido->clear();
    ui->txtDireccion->clear();
    ui->txtTelefono->clear();
    ui->txtEmail->clear();
    if(ui->cbCiudad->count() > 0)
        ui->cbCiudad->setCurrentIndex(0);
    ui->chkActivo->setChecked(true);
    ui->dtFechaNacimiento->setDate(QDate::currentDate());
    ui->txtRucCi->setFocus();
}

void Clientes::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Clientes::on_btnGuardar_clicked()
{
    QString rucCi = ui->txtRucCi->text().trimmed();
    QString nombre = ui->txtNombreApellido->text().trimmed();
    QString direccion = ui->txtDireccion->text().trimmed();
    QString telefono = ui->txtTelefono->text().trimmed();
    QString email = ui->txtEmail->text().trimmed();
    int ciudadId = ui->cbCiudad->currentData().toInt();
    bool activo = ui->chkActivo->isChecked();
    QDate fechaNacimiento = ui->dtFechaNacimiento->date();

    if(rucCi.isEmpty() || nombre.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("RUC/CI y Nombre son obligatorios."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO cliente (ruc_ci, nombre_apellido_cliente, direccion, telefono, email, ciudad_id, activo, fecha_nacimiento) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(rucCi);
    query.addBindValue(nombre);
    query.addBindValue(direccion);
    query.addBindValue(telefono);
    query.addBindValue(email);
    query.addBindValue(ciudadId);
    query.addBindValue(activo);
    query.addBindValue(fechaNacimiento);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el cliente:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Cliente guardado correctamente."));
}

void Clientes::on_btnModificar_clicked()
{
    QString id = ui->txtId->text().trimmed();
    QString rucCi = ui->txtRucCi->text().trimmed();
    QString nombre = ui->txtNombreApellido->text().trimmed();
    QString direccion = ui->txtDireccion->text().trimmed();
    QString telefono = ui->txtTelefono->text().trimmed();
    QString email = ui->txtEmail->text().trimmed();
    int ciudadId = ui->cbCiudad->currentData().toInt();
    bool activo = ui->chkActivo->isChecked();
    QDate fechaNacimiento = ui->dtFechaNacimiento->date();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un cliente para modificar."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE cliente SET ruc_ci=?, nombre_apellido_cliente=?, direccion=?, telefono=?, email=?, ciudad_id=?, activo=?, fecha_nacimiento=? WHERE id=?");
    query.addBindValue(rucCi);
    query.addBindValue(nombre);
    query.addBindValue(direccion);
    query.addBindValue(telefono);
    query.addBindValue(email);
    query.addBindValue(ciudadId);
    query.addBindValue(activo);
    query.addBindValue(fechaNacimiento);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el cliente:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Cliente modificado correctamente."));
}

void Clientes::on_btnEliminar_clicked()
{
    QString id = ui->txtId->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un cliente para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este cliente?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM cliente WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el cliente:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Cliente eliminado correctamente."));
}

void Clientes::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtId->setText(modelo->data(modelo->index(row,0)).toString());
    ui->txtRucCi->setText(modelo->data(modelo->index(row,1)).toString());
    ui->txtNombreApellido->setText(modelo->data(modelo->index(row,2)).toString());
    ui->txtDireccion->setText(modelo->data(modelo->index(row,3)).toString());
    ui->txtTelefono->setText(modelo->data(modelo->index(row,4)).toString());
    ui->txtEmail->setText(modelo->data(modelo->index(row,5)).toString());

    QString ciudadNombre = modelo->data(modelo->index(row,6)).toString();
    int indexCombo = ui->cbCiudad->findText(ciudadNombre);
    if(indexCombo >= 0) ui->cbCiudad->setCurrentIndex(indexCombo);

    ui->chkActivo->setChecked(modelo->data(modelo->index(row,7)).toString() == "Sí");

    QDate fecha = modelo->data(modelo->index(row,8)).toDate().isValid() ?
                      modelo->data(modelo->index(row,8)).toDate() : QDate::currentDate();
    ui->dtFechaNacimiento->setDate(fecha);
}

void Clientes::on_tablaClientes_clicked(const QModelIndex &index)
{
    Q_UNUSED(index);
}

void Clientes::on_btnCerrar_clicked()
{
    close();
}

