#include "menuprincipal.h"
#include "login.h"
#include <QApplication>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    // CONEXIÓN A LA BASE DE DATOS
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");

    QString dbUser = qEnvironmentVariable("DB_USER");
    QString dbPassword = qEnvironmentVariable("DB_PASSWORD");

    QString connString =
        "Driver={PostgreSQL Unicode};"
        "Server=localhost;"
        "Port=5432;"
        "Database=gestion;"
        "Uid=" + dbUser + ";"
        "Pwd=" + dbPassword + ";";

    db.setDatabaseName(connString);


    if (!db.open()) {
        QMessageBox::critical(
            nullptr,
            "Error de Conexión",
            "No se pudo conectar a la base de datos:\n" + db.lastError().text()
            );
        return -1;
    }

    // Bucle principal de la aplicación
    int returnCode = 0;
    bool loginExitoso = true;

    while (loginExitoso) {
        Login loginDialog;

        if (loginDialog.exec() == QDialog::Accepted) {
            // Login exitoso
            MenuPrincipal w;
            w.setUsuarioInfo(
                loginDialog.getUsuarioId(),
                loginDialog.getUsuarioActual(),
                loginDialog.getRolUsuario(),
                loginDialog.getNombreCompleto()
                );
            w.show();
            returnCode = a.exec();

            // Si llegamos aquí, el menú principal se cerró (cerrar sesión)
            // Continuamos el bucle para mostrar login nuevamente
        } else {
            // Login cancelado - salir de la aplicación
            loginExitoso = false;
            returnCode = 0;
        }
    }

    return returnCode;
}
