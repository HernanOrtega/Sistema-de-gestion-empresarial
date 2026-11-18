#ifndef DEPARTAMENTOS_H
#define DEPARTAMENTOS_H

#include <QDialog>
#include <QSqlTableModel>

namespace Ui {
class Departamentos;
}

class Departamentos : public QDialog
{
    Q_OBJECT

public:
    explicit Departamentos(QWidget *parent = nullptr);
    ~Departamentos();

private slots:
    void cargarDatos();                                 // carga la tabla desde la BD
    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnCerrar_clicked();
    void on_tablaDepartamentos_clicked(const QModelIndex &index);

    // optional: manejar selección por teclado/rueda
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous);

private:
    Ui::Departamentos *ui;
    QSqlTableModel *modelo;
    void limpiarCampos();
};

#endif // DEPARTAMENTOS_H
