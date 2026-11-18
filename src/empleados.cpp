#include "empleados.h"
#include "ui_empleados.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Empleados::Empleados(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Empleados),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo azul moderno ------------------------------
    this->setStyleSheet("background-color: #e6f2ff;"); // Fondo azul claro

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #99ccff; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtCedula->setStyleSheet(lineEditStyle);
    ui->txtNombre->setStyleSheet(lineEditStyle);
    ui->txtApellido->setStyleSheet(lineEditStyle);
    ui->txtDireccion->setStyleSheet(lineEditStyle);
    ui->txtTelefono->setStyleSheet(lineEditStyle);
    ui->txtEmail->setStyleSheet(lineEditStyle);
    ui->txtSalario->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #003366; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelCedula->setStyleSheet(labelStyle);
    ui->labelNombre->setStyleSheet(labelStyle);
    ui->labelApellido->setStyleSheet(labelStyle);
    ui->labelDireccion->setStyleSheet(labelStyle);
    ui->labelTelefono->setStyleSheet(labelStyle);
    ui->labelEmail->setStyleSheet(labelStyle);
    ui->labelFechaNacimiento->setStyleSheet(labelStyle);
    ui->labelFechaContratacion->setStyleSheet(labelStyle);
    ui->labelSalario->setStyleSheet(labelStyle);
    ui->labelDepartamento->setStyleSheet(labelStyle);
    ui->labelActivo->setStyleSheet(labelStyle);

    // Estilo para botones grises
    QString btnStyle = R"(
        QPushButton {
            background-color: #666666;
            color: white;
            font-weight: bold;
            border-radius: 6px;
            padding: 6px 12px;
            border: none;
        }
        QPushButton:hover {
            background-color: #555555;
        }
        QPushButton:pressed {
            background-color: #444444;
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
    QString comboStyle = "background-color: #ffffff; color: #000000; border: 1px solid #99ccff; border-radius: 4px; padding: 4px;";
    ui->cbDepartamento->setStyleSheet(comboStyle);

    // Estilo para QDateEdit
    QString dateStyle = "background-color: #ffffff; color: #000000; border: 1px solid #99ccff; border-radius: 4px; padding: 4px;";
    ui->dtFechaNacimiento->setStyleSheet(dateStyle);
    ui->dtFechaContratacion->setStyleSheet(dateStyle);

    // Estilo para QCheckBox
    ui->chkActivo->setStyleSheet("color: #003366; font-weight: bold;");

    // Estilo para QPlainTextEdit
    ui->txtDireccion->setStyleSheet("background-color: #ffffff; color: #000000; border: 1px solid #99ccff; border-radius: 4px; padding: 4px;");

    ui->txtID->setEnabled(false);
    ui->dtFechaNacimiento->setDisplayFormat("yyyy-MM-dd");
    ui->dtFechaContratacion->setDisplayFormat("yyyy-MM-dd");

    // ------------------------------ Configuración tabla gris ------------------------------
    ui->tablaEmpleados->setStyleSheet(R"(
        QTableView {
            background-color: #f8f8f8;
            alternate-background-color: #eeeeee;
            gridline-color: #dddddd;
            selection-background-color: #666666;
            selection-color: white;
            border: 1px solid #cccccc;
            border-radius: 4px;
        }
        QHeaderView::section {
            background-color: #888888;
            color: white;
            font-weight: bold;
            border: 1px solid #666666;
            padding: 6px;
        }
        QTableView::item {
            padding: 4px;
            border-bottom: 1px solid #dddddd;
            color: #333333;
        }
        QTableView::item:selected {
            background-color: #666666;
            color: white;
        }
    )");

    // Primero cargamos departamentos y datos
    cargarDepartamentos();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaEmpleados->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Empleados::filaSeleccionada);
}

Empleados::~Empleados()
{
    delete ui;
}

void Empleados::cargarDepartamentos()
{
    ui->cbDepartamento->clear();
    QSqlQuery query("SELECT id, nombre FROM departamento ORDER BY nombre");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbDepartamento->addItem(nombre, id);
    }
}

void Empleados::cargarDatos()
{
    QString sql = R"(
        SELECT e.id,
               e.cedula,
               e.nombre,
               e.apellido,
               e.direccion,
               e.telefono,
               e.email,
               e.fecha_nacimiento,
               e.fecha_contratacion,
               e.salario,
               d.nombre AS departamento,
               CASE WHEN e.activo THEN 'Sí' ELSE 'No' END AS activo
        FROM empleado e
        LEFT JOIN departamento d ON e.departamento_id = d.id
        ORDER BY e.id
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Cédula"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Nombre"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Apellido"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Dirección"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Teléfono"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Email"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Fecha Nac."));
    modelo->setHeaderData(8, Qt::Horizontal, tr("Fecha Cont."));
    modelo->setHeaderData(9, Qt::Horizontal, tr("Salario"));
    modelo->setHeaderData(10, Qt::Horizontal, tr("Departamento"));
    modelo->setHeaderData(11, Qt::Horizontal, tr("Activo"));

    ui->tablaEmpleados->setModel(modelo);
    ui->tablaEmpleados->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaEmpleados->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaEmpleados->horizontalHeader()->setStretchLastSection(true);
    ui->tablaEmpleados->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Empleados::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtCedula->clear();
    ui->txtNombre->clear();
    ui->txtApellido->clear();
    ui->txtDireccion->clear();
    ui->txtTelefono->clear();
    ui->txtEmail->clear();
    ui->txtSalario->clear();
    if(ui->cbDepartamento->count() > 0)
        ui->cbDepartamento->setCurrentIndex(0);
    ui->chkActivo->setChecked(true);
    ui->dtFechaNacimiento->setDate(QDate::currentDate().addYears(-20));
    ui->dtFechaContratacion->setDate(QDate::currentDate());
    ui->txtCedula->setFocus();
}

void Empleados::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Empleados::on_btnGuardar_clicked()
{
    QString cedula = ui->txtCedula->text().trimmed();
    QString nombre = ui->txtNombre->text().trimmed();
    QString apellido = ui->txtApellido->text().trimmed();
    QString direccion = ui->txtDireccion->toPlainText().trimmed();
    QString telefono = ui->txtTelefono->text().trimmed();
    QString email = ui->txtEmail->text().trimmed();
    QDate fechaNacimiento = ui->dtFechaNacimiento->date();
    QDate fechaContratacion = ui->dtFechaContratacion->date();
    double salario = ui->txtSalario->text().toDouble();
    int departamentoId = ui->cbDepartamento->currentData().toInt();
    bool activo = ui->chkActivo->isChecked();

    if(cedula.isEmpty() || nombre.isEmpty() || apellido.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Cédula, Nombre y Apellido son obligatorios."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO empleado (cedula, nombre, apellido, direccion, telefono, email, fecha_nacimiento, fecha_contratacion, salario, departamento_id, activo) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(cedula);
    query.addBindValue(nombre);
    query.addBindValue(apellido);
    query.addBindValue(direccion);
    query.addBindValue(telefono);
    query.addBindValue(email);
    query.addBindValue(fechaNacimiento);
    query.addBindValue(fechaContratacion);
    query.addBindValue(salario);
    query.addBindValue(departamentoId);
    query.addBindValue(activo);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el empleado:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Empleado guardado correctamente."));
}

void Empleados::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    QString cedula = ui->txtCedula->text().trimmed();
    QString nombre = ui->txtNombre->text().trimmed();
    QString apellido = ui->txtApellido->text().trimmed();
    QString direccion = ui->txtDireccion->toPlainText().trimmed();
    QString telefono = ui->txtTelefono->text().trimmed();
    QString email = ui->txtEmail->text().trimmed();
    QDate fechaNacimiento = ui->dtFechaNacimiento->date();
    QDate fechaContratacion = ui->dtFechaContratacion->date();
    double salario = ui->txtSalario->text().toDouble();
    int departamentoId = ui->cbDepartamento->currentData().toInt();
    bool activo = ui->chkActivo->isChecked();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un empleado para modificar."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE empleado SET cedula=?, nombre=?, apellido=?, direccion=?, telefono=?, email=?, fecha_nacimiento=?, fecha_contratacion=?, salario=?, departamento_id=?, activo=? WHERE id=?");
    query.addBindValue(cedula);
    query.addBindValue(nombre);
    query.addBindValue(apellido);
    query.addBindValue(direccion);
    query.addBindValue(telefono);
    query.addBindValue(email);
    query.addBindValue(fechaNacimiento);
    query.addBindValue(fechaContratacion);
    query.addBindValue(salario);
    query.addBindValue(departamentoId);
    query.addBindValue(activo);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el empleado:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Empleado modificado correctamente."));
}

void Empleados::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un empleado para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este empleado?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM empleado WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el empleado:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Empleado eliminado correctamente."));
}

void Empleados::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());
    ui->txtCedula->setText(modelo->data(modelo->index(row,1)).toString());
    ui->txtNombre->setText(modelo->data(modelo->index(row,2)).toString());
    ui->txtApellido->setText(modelo->data(modelo->index(row,3)).toString());
    ui->txtDireccion->setPlainText(modelo->data(modelo->index(row,4)).toString());
    ui->txtTelefono->setText(modelo->data(modelo->index(row,5)).toString());
    ui->txtEmail->setText(modelo->data(modelo->index(row,6)).toString());

    QDate fechaNac = modelo->data(modelo->index(row,7)).toDate().isValid() ?
                         modelo->data(modelo->index(row,7)).toDate() : QDate::currentDate().addYears(-20);
    ui->dtFechaNacimiento->setDate(fechaNac);

    QDate fechaCont = modelo->data(modelo->index(row,8)).toDate().isValid() ?
                          modelo->data(modelo->index(row,8)).toDate() : QDate::currentDate();
    ui->dtFechaContratacion->setDate(fechaCont);

    ui->txtSalario->setText(modelo->data(modelo->index(row,9)).toString());

    QString departamentoNombre = modelo->data(modelo->index(row,10)).toString();
    int indexCombo = ui->cbDepartamento->findText(departamentoNombre);
    if(indexCombo >= 0) ui->cbDepartamento->setCurrentIndex(indexCombo);

    ui->chkActivo->setChecked(modelo->data(modelo->index(row,11)).toString() == "Sí");
}

void Empleados::on_btnSalir_clicked()
{
    close();
}
