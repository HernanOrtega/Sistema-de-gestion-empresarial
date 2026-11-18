#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

    // Métodos para obtener información del usuario logueado
    QString getUsuarioActual() const;
    QString getRolUsuario() const;
    QString getNombreCompleto() const;
    int getUsuarioId() const;

private slots:
    void on_btnLogin_clicked();
    void on_btnCancelar_clicked();

private:
    Ui::Login *ui;
    int usuarioId;
    QString username;
    QString rol;
    QString nombreCompleto;

    bool verificarCredenciales(const QString &username, const QString &password);
    void actualizarUltimoLogin(int usuarioId);
};

#endif // LOGIN_H
