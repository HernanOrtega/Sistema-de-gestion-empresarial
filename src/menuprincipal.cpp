#include "menuprincipal.h"
#include "ui_menuprincipal.h"
#include "login.h"
#include "departamentos.h"
#include "ciudades.h"
#include "monedas.h"
#include "marcas.h"
#include "proveedores.h"
#include "clientes.h"
#include "productos.h"
#include "empleados.h"
#include "usuarios.h"
#include "caja.h"
#include "cobrotipo.h"
#include "cobros.h"
#include "detallecobros.h"
#include "arqueos.h"
#include "pedidosclientes.h"
#include "detallepedidosclientes.h"
#include "ventas.h"
#include "detalleventas.h"
#include "ventaspedidos.h"
#include "presupuestos.h"
#include "detallepresupuestos.h"
#include "cotizaciones.h"
#include "pedidosproveedores.h"
#include "detallepedidosproveedores.h"
#include "ordenescompra.h"
#include "detalleordenescompra.h"
#include "compras.h"
#include "detallecompras.h"
#include "ordenespago.h"
#include "detalleordenespago.h"
#include "pagos.h"
#include "cuentascobrar.h"
#include "cuentaspagar.h"
#include "stock.h"
#include "tipoajustes.h"
#include "ajustes.h"
#include "detalleajustes.h"
#include "devolucionesproveedores.h"
#include "detalledevolucionesproveedores.h"
#include "auditoria.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QResizeEvent>
#include <QApplication>

MenuPrincipal::MenuPrincipal(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MenuPrincipal)
    , usuarioId(-1)
{
    ui->setupUi(this);
    this->showMaximized();  // Pantalla completa al iniciar

    // -------------------------------------------------------------
    // CENTRAR EL LABEL DE TÍTULO
    // -------------------------------------------------------------
    ui->labelTitulo->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->labelTitulo->setGeometry(0, 0, this->width(), 60);

    // -------------------------------------------------------------
    // CONFIGURAR LABEL LOGIN Y BOTÓN CERRAR SESIÓN
    // -------------------------------------------------------------
    // Estilo para el label de login (verde, pequeño)
    ui->labelLogin->setStyleSheet("color: #00aa00; font-weight: bold; font-size: 12px; background-color: transparent;");
    ui->labelLogin->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // Estilo para el botón cerrar sesión (ROJO como solicitaste)
    QString btnCerrarSesionStyle = R"(
        QPushButton {
            background-color: #ff4444;
            color: white;
            font-weight: bold;
            border-radius: 4px;
            padding: 6px 12px;
            border: none;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #cc0000;
        }
        QPushButton:pressed {
            background-color: #aa0000;
        }
    )";
    ui->btnSalirSistema->setStyleSheet(btnCerrarSesionStyle);
    ui->btnSalirSistema->setText("Cerrar Sesión");

    // Estilo global para QMessageBox (fondo blanco, texto negro)
    qApp->setStyleSheet("QMessageBox { "
                        "background-color: white; "
                        "color: black; "
                        "}"

                        "QMessageBox QLabel { "
                        "color: black; "
                        "}"

                        "QMessageBox QPushButton { "
                        "background-color: #f0f0f0; "
                        "color: black; "
                        "padding: 5px 15px; "
                        "border: 1px solid #cccccc; "
                        "border-radius: 3px; "
                        "}"

                        "QMessageBox QPushButton:hover { "
                        "background-color: #e0e0e0; "
                        "}"

                        "QMessageBox QPushButton:pressed { "
                        "background-color: #d0d0d0; "
                        "}");

    // Posicionar inicialmente
    actualizarLabelLogin();

    // Mensaje inicial en el labelTitulo
    ui->labelTitulo->setText("Sistema de Gestión - Inicie Sesión");
}

MenuPrincipal::~MenuPrincipal()
{
    delete ui;
}

void MenuPrincipal::setUsuarioInfo(int id, const QString &username, const QString &rol, const QString &nombreCompleto)
{
    usuarioId = id;
    usuarioUsername = username;
    usuarioRol = rol;
    usuarioNombreCompleto = nombreCompleto;

    // Actualizar label de login
    actualizarLabelLogin();

    qDebug() << "Usuario logueado:" << nombreCompleto << "Rol:" << rol << "ID:" << id;
}

bool MenuPrincipal::verificarPermiso(const QString &modulo)
{
    // Si no hay usuario logueado, denegar acceso
    if (usuarioId <= 0) {
        qDebug() << "ERROR: No hay usuario logueado!";
        return false;
    }

    // Admin tiene acceso completo a todo
    if (usuarioRol == "admin") {
        return true;
    }

    // Mapa de permisos por rol y módulo
    QHash<QString, QStringList> permisos;

    // Definir permisos para cada rol
    permisos["gerente"] = QStringList()
                          << "departamentos" << "ciudades" << "monedas" << "marcas"
                          << "proveedores" << "clientes" << "productos" << "empleados"
                          << "caja" << "cobrostipo" << "cobros" << "detallecobros" << "arqueos"
                          << "pedidosclientes" << "detallepedidosclientes" << "ventas" << "detalleventas"
                          << "ventaspedidos" << "presupuestos" << "detallepresupuestos" << "cotizaciones"
                          << "pedidosproveedores" << "detallepedidosproveedores" << "ordenescompra"
                          << "detalleordenescompra" << "compras" << "detallecompras" << "ordenespago"
                          << "detalleordenespago" << "pagos" << "cuentascobrar" << "cuentaspagar"
                          << "stock" << "tipoajustes" << "ajustes" << "detalleajustes"
                          << "devolucionesproveedores" << "detalledevolucionesproveedores" << "auditoria";

    permisos["vendedor"] = QStringList()
                           << "clientes" << "productos" << "cobros" << "detallecobros"
                           << "pedidosclientes" << "detallepedidosclientes" << "ventas" << "detalleventas"
                           << "ventaspedidos" << "presupuestos" << "detallepresupuestos" << "cotizaciones"
                           << "cuentascobrar";

    permisos["almacen"] = QStringList()
                          << "marcas" << "proveedores" << "productos" << "pedidosproveedores"
                          << "detallepedidosproveedores" << "ordenescompra" << "detalleordenescompra"
                          << "compras" << "detallecompras" << "stock" << "tipoajustes" << "ajustes"
                          << "detalleajustes" << "devolucionesproveedores" << "detalledevolucionesproveedores";

    // Verificar permisos para roles específicos
    if(permisos.contains(usuarioRol)) {
        return permisos[usuarioRol].contains(modulo);
    }

    qDebug() << "Rol no reconocido o sin permisos:" << usuarioRol;
    return false;
}

void MenuPrincipal::mostrarMensajeSinPermiso()
{
    QMessageBox::warning(this, "Acceso Denegado",
                         "Atención. No tienes permiso para acceder a esta sección.\n\n"
                         "Contacta al administrador del sistema si necesitas acceso a este módulo.");
}

void MenuPrincipal::actualizarLabelLogin()
{
    if (usuarioId > 0) {
        // Usuario logueado - mostrar información en labelLogin
        QString loginText = QString("Conectado como: %1 (%2)").arg(usuarioNombreCompleto).arg(usuarioRol);
        ui->labelLogin->setText(loginText);
        ui->btnSalirSistema->setVisible(true);

        // Mantener el labelTitulo con texto simple
        ui->labelTitulo->setText("Sistema de Gestión");
    } else {
        // No hay usuario logueado
        ui->labelLogin->setText("No conectado");
        ui->btnSalirSistema->setVisible(false);
        ui->labelTitulo->setText("Sistema de Gestión - Inicie Sesión");
    }
}

// -------------------------------------------------------------
// Mantener elementos posicionados correctamente al redimensionar
// -------------------------------------------------------------
void MenuPrincipal::resizeEvent(QResizeEvent *event)
{
    // Título centrado
    ui->labelTitulo->setGeometry(0, 0, this->width(), 60);

    // Label login y botón cerrar sesión en esquina superior derecha
    int labelWidth = 250;
    int buttonWidth = 120;
    int rightMargin = 20;
    int topMargin = 10;

    ui->labelLogin->setGeometry(this->width() - labelWidth - buttonWidth - rightMargin - 10,
                                topMargin, labelWidth, 25);
    ui->btnSalirSistema->setGeometry(this->width() - buttonWidth - rightMargin,
                                     topMargin, buttonWidth, 25);

    QMainWindow::resizeEvent(event);
}

// -------------------------------------------------------------
// BOTÓN CERRAR SESIÓN (REEMPLAZA AL BOTÓN SALIR)
// -------------------------------------------------------------
void MenuPrincipal::on_btnSalirSistema_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Cerrar Sesión");
    msgBox.setText("¿Está seguro que desea cerrar la sesión actual?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    // Aplicar estilo específico para este messagebox
    msgBox.setStyleSheet("QMessageBox { background-color: #2b2b2b; color: white; } "
                         "QMessageBox QLabel { color: white; } "
                         "QMessageBox QPushButton { background-color: #555555; color: white; padding: 5px 15px; border: none; border-radius: 3px; } "
                         "QMessageBox QPushButton:hover { background-color: #666666; }");

    if (msgBox.exec() == QMessageBox::Yes) {
        // Cerrar esta ventana (volverá al main.cpp que mostrará el login nuevamente)
        this->close();
    }
}

// -------------------------------------------------------------
// MÉTODOS PARA ABRIR FORMULARIOS CON VERIFICACIÓN DE PERMISOS
// -------------------------------------------------------------

void MenuPrincipal::on_btnDepartamentos_clicked()
{
    if (!verificarPermiso("departamentos")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Departamentos *form = new Departamentos(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnCiudades_clicked()
{
    if (!verificarPermiso("ciudades")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Ciudades *form = new Ciudades(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnMonedas_clicked()
{
    if (!verificarPermiso("monedas")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Monedas *form = new Monedas(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnMarcas_clicked()
{
    if (!verificarPermiso("marcas")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Marcas *form = new Marcas(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnProveedores_clicked()
{
    if (!verificarPermiso("proveedores")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Proveedores *form = new Proveedores(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnClientes_clicked()
{
    if (!verificarPermiso("clientes")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Clientes *form = new Clientes(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnProductos_clicked()
{
    if (!verificarPermiso("productos")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Productos *form = new Productos(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnEmpleados_clicked()
{
    if (!verificarPermiso("empleados")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Empleados *form = new Empleados(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnUsuarios_clicked()
{
    if (!verificarPermiso("usuarios")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Usuarios *form = new Usuarios(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnCaja_clicked()
{
    if (!verificarPermiso("caja")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Caja *form = new Caja(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnCobrosTipo_clicked()
{
    if (!verificarPermiso("cobrostipo")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Cobrotipo *form = new Cobrotipo(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnCobros_clicked()
{
    if (!verificarPermiso("cobros")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Cobros *form = new Cobros(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnDetalleCobros_clicked()
{
    if (!verificarPermiso("detallecobros")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Detallecobros *form = new Detallecobros(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnArqueos_clicked()
{
    if (!verificarPermiso("arqueos")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Arqueos *form = new Arqueos(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnPedidosClientes_clicked()
{
    if (!verificarPermiso("pedidosclientes")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Pedidosclientes *form = new Pedidosclientes(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnDetallePedidosClientes_clicked()
{
    if (!verificarPermiso("detallepedidosclientes")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Detallepedidosclientes *form = new Detallepedidosclientes(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnVentas_clicked()
{
    if (!verificarPermiso("ventas")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Ventas *form = new Ventas(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnDetalleVentas_clicked()
{
    if (!verificarPermiso("detalleventas")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Detalleventas *form = new Detalleventas(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnVentasPedidos_clicked()
{
    if (!verificarPermiso("ventaspedidos")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Ventaspedidos *form = new Ventaspedidos(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnPresupuestos_clicked()
{
    if (!verificarPermiso("presupuestos")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Presupuestos *form = new Presupuestos(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnDetallePresupuestos_clicked()
{
    if (!verificarPermiso("detallepresupuestos")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Detallepresupuestos *form = new Detallepresupuestos(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnCotizaciones_clicked()
{
    if (!verificarPermiso("cotizaciones")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Cotizaciones *form = new Cotizaciones(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnPedidosProveedores_clicked()
{
    if (!verificarPermiso("pedidosproveedores")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Pedidosproveedores *form = new Pedidosproveedores(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnDetallePedidosProveedores_clicked()
{
    if (!verificarPermiso("detallepedidosproveedores")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Detallepedidosproveedores *form = new Detallepedidosproveedores(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnOrdenesCompra_clicked()
{
    if (!verificarPermiso("ordenescompra")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Ordenescompra *form = new Ordenescompra(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnDetalleOrdenesCompra_clicked()
{
    if (!verificarPermiso("detalleordenescompra")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Detalleordenescompra *form = new Detalleordenescompra(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnCompras_clicked()
{
    if (!verificarPermiso("compras")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Compras *form = new Compras(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnDetalleCompras_clicked()
{
    if (!verificarPermiso("detallecompras")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Detallecompras *form = new Detallecompras(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnOrdenesPago_clicked()
{
    if (!verificarPermiso("ordenespago")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Ordenespago *form = new Ordenespago(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnDetalleOrdenesPago_clicked()
{
    if (!verificarPermiso("detalleordenespago")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Detalleordenespago *form = new Detalleordenespago(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnPagos_clicked()
{
    if (!verificarPermiso("pagos")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Pagos *form = new Pagos(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnCuentasCobrar_clicked()
{
    if (!verificarPermiso("cuentascobrar")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Cuentascobrar *form = new Cuentascobrar(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnCuentasPagar_clicked()
{
    if (!verificarPermiso("cuentaspagar")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Cuentaspagar *form = new Cuentaspagar(this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnStock_clicked()
{
    if (!verificarPermiso("stock")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Stock *form = new Stock (this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnTipoAjustes_clicked()
{
    if (!verificarPermiso("tipoajustes")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Tipoajustes *form = new Tipoajustes (this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnAjustes_clicked()
{
    if (!verificarPermiso("ajustes")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Ajustes *form = new Ajustes (this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnDetalleAjustes_clicked()
{
    if (!verificarPermiso("detalleajustes")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Detalleajustes *form = new Detalleajustes (this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnDevolucionesProveedores_clicked()
{
    if (!verificarPermiso("devolucionesproveedores")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Devolucionesproveedores *form = new Devolucionesproveedores (this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnDetalleDevolucionesProveedores_clicked()
{
    if (!verificarPermiso("detalledevolucionesproveedores")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Detalledevolucionesproveedores *form = new Detalledevolucionesproveedores (this);
    form->setModal(true);
    form->exec();
}

void MenuPrincipal::on_btnAuditoria_clicked()
{
    if (!verificarPermiso("auditoria")) {
        mostrarMensajeSinPermiso();
        return;
    }
    Auditoria *form = new Auditoria (this);
    form->setModal(true);
    form->exec();
}
