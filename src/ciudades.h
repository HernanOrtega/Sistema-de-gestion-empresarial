#ifndef CIUDADES_H
#define CIUDADES_H

#include <QDialog>
#include <QSqlRelationalTableModel>

namespace Ui {
class Ciudades;
}

class Ciudades : public QDialog
{
    Q_OBJECT

public:
    explicit Ciudades(QWidget *parent = nullptr);
    ~Ciudades();

private slots:
    void cargarDatos();                                 // carga la tabla desde la BD
    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnCerrar_clicked();
    void on_tablaCiudades_clicked(const QModelIndex &index);
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous);

private:
    Ui::Ciudades *ui;
    QSqlRelationalTableModel *modelo;                  // CAMBIO: ahora es relacional
    void limpiarCampos();
    void cargarDepartamentos();                        // llena el comboBox con los departamentos
};

#endif // CIUDADES_H
