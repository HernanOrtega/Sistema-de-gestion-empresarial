#include "login.h"
#include "ui_login.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDebug>

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login),
    usuarioId(-1)
{
    ui->setupUi(this);

    // Estilo del formulario de login
    this->setStyleSheet("background-color: #e6f2ff;");
    setWindowTitle("Sistema de Gestión - Login");

    // Estilo para los campos
    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #99ccff; border-radius: 4px; padding: 8px; font-size: 14px;";
    ui->txtUsername->setStyleSheet(lineEditStyle);
    ui->txtPassword->setStyleSheet(lineEditStyle);

    // Estilo para los botones
    QString btnStyle = R"(
        QPushButton {
            background-color: #666666;
            color: white;
            font-weight: bold;
            border-radius: 6px;
            padding: 10px 20px;
            border: none;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #555555;
        }
        QPushButton:pressed {
            background-color: #444444;
        }
    )";

    ui->btnLogin->setStyleSheet(btnStyle);
    ui->btnCancelar->setStyleSheet(btnStyle);

    // Estilo para labels
    QString labelStyle = "color: #003366; font-weight: bold; font-size: 14px;";
    ui->labelUsername->setStyleSheet(labelStyle);
    ui->labelPassword->setStyleSheet(labelStyle);
    ui->labelTitulo->setStyleSheet("color: #003366; font-weight: bold; font-size: 18px; qproperty-alignment: AlignCenter;");

    ui->txtPassword->setEchoMode(QLineEdit::Password);

    // Conectar Enter para login
    connect(ui->txtPassword, &QLineEdit::returnPressed, this, &Login::on_btnLogin_clicked);
    connect(ui->txtUsername, &QLineEdit::returnPressed, this, &Login::on_btnLogin_clicked);
}

Login::~Login()
{
    delete ui;
}

// Nuevos métodos getter
QString Login::getUsuarioActual() const {
    return username;
}

QString Login::getRolUsuario() const {
    return rol;
}

QString Login::getNombreCompleto() const {
    return nombreCompleto;
}

int Login::getUsuarioId() const {
    return usuarioId;
}

bool Login::verificarCredenciales(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare(
        "SELECT u.id, u.password_hash, u.rol, u.activo, "
        "COALESCE(e.nombre || ' ' || e.apellido, u.username) as nombre_completo "
        "FROM usuario u "
        "LEFT JOIN empleado e ON u.empleado_id = e.id "
        "WHERE u.username = ?"
        );
    query.addBindValue(username);

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "Error al verificar credenciales:\n" + query.lastError().text());
        return false;
    }

    if (!query.next()) {
        QMessageBox::warning(this, "Login fallido", "Usuario no encontrado.");
        return false;
    }

    // Verificar si el usuario está activo
    bool activo = query.value(3).toBool();
    if (!activo) {
        QMessageBox::warning(this, "Login fallido", "El usuario está desactivado.");
        return false;
    }

    // Verificar contraseña
    QString storedHash = query.value(1).toString();
    QString inputHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    if (inputHash != storedHash) {
        QMessageBox::warning(this, "Login fallido", "Contraseña incorrecta.");
        return false;
    }

    // Guardar información del usuario
    usuarioId = query.value(0).toInt();
    this->username = username;
    rol = query.value(2).toString();
    nombreCompleto = query.value(4).toString();

    return true;
}

void Login::actualizarUltimoLogin(int usuarioId)
{
    QSqlQuery query;
    query.prepare("UPDATE usuario SET ultimo_login = NOW() WHERE id = ?");
    query.addBindValue(usuarioId);
    query.exec(); // No es crítico si falla, así que no mostramos error
}

void Login::on_btnLogin_clicked()
{
    QString username = ui->txtUsername->text().trimmed();
    QString password = ui->txtPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Validación", "Por favor ingrese usuario y contraseña.");
        return;
    }

    if (verificarCredenciales(username, password)) {
        // Actualizar último login
        actualizarUltimoLogin(usuarioId);

        // Aceptar el diálogo (login exitoso)
        accept();
    }
}

void Login::on_btnCancelar_clicked()
{
    reject(); // Cerrar aplicación
}
