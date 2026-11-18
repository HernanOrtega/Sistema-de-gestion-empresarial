#include "usuarios.h"
#include "ui_usuarios.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QCryptographicHash>

Usuarios::Usuarios(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Usuarios),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo azul moderno ------------------------------
    this->setStyleSheet("background-color: #e6f2ff;"); // Fondo azul claro

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #99ccff; border-radius: 4px; padding: 4px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtUsername->setStyleSheet(lineEditStyle);
    ui->txtPassword->setStyleSheet(lineEditStyle);
    ui->txtConfirmarPassword->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #003366; font-weight: bold;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelEmpleado->setStyleSheet(labelStyle);
    ui->labelUsername->setStyleSheet(labelStyle);
    ui->labelPassword->setStyleSheet(labelStyle);
    ui->labelConfirmarPassword->setStyleSheet(labelStyle);
    ui->labelRol->setStyleSheet(labelStyle);
    ui->labelActivo->setStyleSheet(labelStyle);
    ui->labelFechaCreacion->setStyleSheet(labelStyle);
    ui->labelUltimoLogin->setStyleSheet(labelStyle);

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
    ui->cbEmpleado->setStyleSheet(comboStyle);
    ui->cbRol->setStyleSheet(comboStyle);

    // Estilo para QDateTimeEdit (readonly)
    QString dateTimeStyle = "background-color: #f0f0f0; color: #666666; border: 1px solid #99ccff; border-radius: 4px; padding: 4px;";
    ui->dtFechaCreacion->setStyleSheet(dateTimeStyle);
    ui->dtUltimoLogin->setStyleSheet(dateTimeStyle);

    // Estilo para QCheckBox
    ui->chkActivo->setStyleSheet("color: #003366; font-weight: bold;");

    ui->txtID->setEnabled(false);
    ui->txtPassword->setEchoMode(QLineEdit::Password);
    ui->txtConfirmarPassword->setEchoMode(QLineEdit::Password);

    // Configurar QDateTimeEdit como readonly
    ui->dtFechaCreacion->setEnabled(false);
    ui->dtUltimoLogin->setEnabled(false);

    // Formato de fecha y hora
    ui->dtFechaCreacion->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtUltimoLogin->setDisplayFormat("yyyy-MM-dd HH:mm");

    // Llenar combo de roles
    ui->cbRol->clear();
    ui->cbRol->addItem("Administrador", "admin");
    ui->cbRol->addItem("Vendedor", "vendedor");
    ui->cbRol->addItem("Gerente", "gerente");
    ui->cbRol->addItem("Almacen", "almacen");

    // ------------------------------ Configuración tabla gris ------------------------------
    ui->tablaUsuarios->setStyleSheet(R"(
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

    // Primero cargamos empleados y datos
    cargarEmpleados();
    cargarDatos();

    // Conectar la señal después de setModel
    connect(ui->tablaUsuarios->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &Usuarios::filaSeleccionada);
}

Usuarios::~Usuarios()
{
    delete ui;
}

void Usuarios::cargarEmpleados()
{
    ui->cbEmpleado->clear();
    ui->cbEmpleado->addItem("-- Sin empleado --", 0); // Opción para usuarios sin empleado asociado

    QSqlQuery query("SELECT id, nombre || ' ' || apellido AS nombre_completo FROM empleado WHERE activo = true ORDER BY nombre, apellido");
    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombreCompleto = query.value(1).toString();
        ui->cbEmpleado->addItem(nombreCompleto, id);
    }
}

void Usuarios::cargarDatos()
{
    QString sql = R"(
        SELECT u.id,
               COALESCE(e.nombre || ' ' || e.apellido, '-- Sin empleado --') AS empleado,
               u.username,
               u.rol,
               CASE WHEN u.activo THEN 'Sí' ELSE 'No' END AS activo,
               u.fecha_creacion,
               u.ultimo_login
        FROM usuario u
        LEFT JOIN empleado e ON u.empleado_id = e.id
        ORDER BY u.id
    )";

    modelo->setQuery(sql);

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Empleado"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Username"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Rol"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Activo"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Fecha Creación"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Último Login"));

    ui->tablaUsuarios->setModel(modelo);
    ui->tablaUsuarios->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaUsuarios->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaUsuarios->horizontalHeader()->setStretchLastSection(true);
    ui->tablaUsuarios->setEditTriggers(QAbstractItemView::NoEditTriggers);

    limpiarCampos();
}

void Usuarios::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtUsername->clear();
    ui->txtPassword->clear();
    ui->txtConfirmarPassword->clear();
    if(ui->cbEmpleado->count() > 0)
        ui->cbEmpleado->setCurrentIndex(0);
    if(ui->cbRol->count() > 0)
        ui->cbRol->setCurrentIndex(0);
    ui->chkActivo->setChecked(true);

    // Limpiar fechas
    ui->dtFechaCreacion->setDateTime(QDateTime::currentDateTime());
    ui->dtUltimoLogin->setDateTime(QDateTime::currentDateTime());

    ui->txtUsername->setFocus();
}

void Usuarios::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Usuarios::on_btnGuardar_clicked()
{
    QString username = ui->txtUsername->text().trimmed();
    QString password = ui->txtPassword->text();
    QString confirmarPassword = ui->txtConfirmarPassword->text();
    int empleadoId = ui->cbEmpleado->currentData().toInt();
    QString rol = ui->cbRol->currentData().toString();
    bool activo = ui->chkActivo->isChecked();

    if(username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Username y Password son obligatorios."));
        return;
    }

    if(password != confirmarPassword) {
        QMessageBox::warning(this, tr("Atención"), tr("Las contraseñas no coinciden."));
        return;
    }

    if(password.length() < 4) {
        QMessageBox::warning(this, tr("Atención"), tr("La contraseña debe tener al menos 4 caracteres."));
        return;
    }

    // Hash de la contraseña
    QString passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    QSqlQuery query;
    query.prepare("INSERT INTO usuario (empleado_id, username, password_hash, rol, activo) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(empleadoId > 0 ? empleadoId : QVariant());
    query.addBindValue(username);
    query.addBindValue(passwordHash);
    query.addBindValue(rol);
    query.addBindValue(activo);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el usuario:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Usuario guardado correctamente."));
}

void Usuarios::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    QString username = ui->txtUsername->text().trimmed();
    QString password = ui->txtPassword->text();
    int empleadoId = ui->cbEmpleado->currentData().toInt();
    QString rol = ui->cbRol->currentData().toString();
    bool activo = ui->chkActivo->isChecked();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un usuario para modificar."));
        return;
    }

    if(username.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Username es obligatorio."));
        return;
    }

    QSqlQuery query;

    // Si se ingresó una nueva contraseña, actualizarla
    if(!password.isEmpty()) {
        QString passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
        query.prepare("UPDATE usuario SET empleado_id=?, username=?, password_hash=?, rol=?, activo=? WHERE id=?");
        query.addBindValue(empleadoId > 0 ? empleadoId : QVariant());
        query.addBindValue(username);
        query.addBindValue(passwordHash);
        query.addBindValue(rol);
        query.addBindValue(activo);
        query.addBindValue(id.toInt());
    } else {
        // Si no se cambió la contraseña, mantener la actual
        query.prepare("UPDATE usuario SET empleado_id=?, username=?, rol=?, activo=? WHERE id=?");
        query.addBindValue(empleadoId > 0 ? empleadoId : QVariant());
        query.addBindValue(username);
        query.addBindValue(rol);
        query.addBindValue(activo);
        query.addBindValue(id.toInt());
    }

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el usuario:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Usuario modificado correctamente."));
}

void Usuarios::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Selecciona un usuario para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este usuario?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM usuario WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el usuario:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Usuario eliminado correctamente."));
}

void Usuarios::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    QString id = modelo->data(modelo->index(row,0)).toString();
    ui->txtID->setText(id);
    ui->txtUsername->setText(modelo->data(modelo->index(row,2)).toString());

    // No cargamos la contraseña por seguridad
    ui->txtPassword->clear();
    ui->txtConfirmarPassword->clear();

    QString rolDisplay = modelo->data(modelo->index(row,3)).toString();
    int indexRol = ui->cbRol->findText(rolDisplay);
    if(indexRol >= 0) ui->cbRol->setCurrentIndex(indexRol);

    QString empleadoNombre = modelo->data(modelo->index(row,1)).toString();
    int indexEmpleado = ui->cbEmpleado->findText(empleadoNombre);
    if(indexEmpleado >= 0) {
        ui->cbEmpleado->setCurrentIndex(indexEmpleado);
    } else {
        ui->cbEmpleado->setCurrentIndex(0); // "-- Sin empleado --"
    }

    ui->chkActivo->setChecked(modelo->data(modelo->index(row,4)).toString() == "Sí");

    // Cargar fechas desde el modelo
    QDateTime fechaCreacion = modelo->data(modelo->index(row,5)).toDateTime();
    QDateTime ultimoLogin = modelo->data(modelo->index(row,6)).toDateTime();

    if (fechaCreacion.isValid()) {
        ui->dtFechaCreacion->setDateTime(fechaCreacion);
    } else {
        ui->dtFechaCreacion->setDateTime(QDateTime::currentDateTime());
    }

    if (ultimoLogin.isValid()) {
        ui->dtUltimoLogin->setDateTime(ultimoLogin);
    } else {
        ui->dtUltimoLogin->setDateTime(QDateTime::currentDateTime());
    }
}

void Usuarios::on_btnSalir_clicked()
{
    close();
}
