#ifndef MENUPRINCIPAL_H
#define MENUPRINCIPAL_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MenuPrincipal;
}
QT_END_NAMESPACE

class MenuPrincipal : public QMainWindow
{
    Q_OBJECT

public:
    MenuPrincipal(QWidget *parent = nullptr);
    ~MenuPrincipal();

    // Método para establecer información del usuario logueado
    void setUsuarioInfo(int id, const QString &username, const QString &rol, const QString &nombreCompleto);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_btnDepartamentos_clicked();
    void on_btnCiudades_clicked();
    void on_btnMonedas_clicked();
    void on_btnMarcas_clicked();
    void on_btnProveedores_clicked();
    void on_btnClientes_clicked();
    void on_btnProductos_clicked();
    void on_btnEmpleados_clicked();
    void on_btnUsuarios_clicked();
    void on_btnCaja_clicked();
    void on_btnCobrosTipo_clicked();
    void on_btnCobros_clicked();
    void on_btnDetalleCobros_clicked();
    void on_btnArqueos_clicked();
    void on_btnPedidosClientes_clicked();
    void on_btnDetallePedidosClientes_clicked();
    void on_btnVentas_clicked();
    void on_btnDetalleVentas_clicked();
    void on_btnVentasPedidos_clicked();
    void on_btnPresupuestos_clicked();
    void on_btnDetallePresupuestos_clicked();
    void on_btnCotizaciones_clicked();
    void on_btnPedidosProveedores_clicked();
    void on_btnDetallePedidosProveedores_clicked();
    void on_btnOrdenesCompra_clicked();
    void on_btnDetalleOrdenesCompra_clicked();
    void on_btnCompras_clicked();
    void on_btnDetalleCompras_clicked();
    void on_btnOrdenesPago_clicked();
    void on_btnDetalleOrdenesPago_clicked();
    void on_btnPagos_clicked();
    void on_btnCuentasCobrar_clicked();
    void on_btnCuentasPagar_clicked();
    void on_btnStock_clicked();
    void on_btnTipoAjustes_clicked();
    void on_btnAjustes_clicked();
    void on_btnDetalleAjustes_clicked();
    void on_btnDevolucionesProveedores_clicked();
    void on_btnDetalleDevolucionesProveedores_clicked();
    void on_btnAuditoria_clicked();
    void on_btnSalirSistema_clicked();  // Mantener el nombre original del botón

private:
    Ui::MenuPrincipal *ui;
    int usuarioId;
    QString usuarioUsername;
    QString usuarioRol;
    QString usuarioNombreCompleto;

    void actualizarInterfazSegunRol();
    void actualizarLabelLogin();
    void configurarRestricciones();
    bool verificarPermiso(const QString &modulo);
    void mostrarMensajeSinPermiso();
};

#endif // MENUPRINCIPAL_H
