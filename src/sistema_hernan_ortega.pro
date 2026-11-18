QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ajustes.cpp \
    arqueos.cpp \
    auditoria.cpp \
    caja.cpp \
    ciudades.cpp \
    clientes.cpp \
    cobros.cpp \
    cobrotipo.cpp \
    compras.cpp \
    cotizaciones.cpp \
    cuentascobrar.cpp \
    cuentaspagar.cpp \
    departamentos.cpp \
    detalleajustes.cpp \
    detallecobros.cpp \
    detallecompras.cpp \
    detalledevolucionesproveedores.cpp \
    detalleordenescompra.cpp \
    detalleordenespago.cpp \
    detallepedidosclientes.cpp \
    detallepedidosproveedores.cpp \
    detallepresupuestos.cpp \
    detalleventas.cpp \
    devolucionesproveedores.cpp \
    empleados.cpp \
    login.cpp \
    main.cpp \
    marcas.cpp \
    menuprincipal.cpp \
    monedas.cpp \
    ordenescompra.cpp \
    ordenespago.cpp \
    pagos.cpp \
    pedidosclientes.cpp \
    pedidosproveedores.cpp \
    presupuestos.cpp \
    productos.cpp \
    proveedores.cpp \
    stock.cpp \
    tipoajustes.cpp \
    usuarios.cpp \
    ventas.cpp \
    ventaspedidos.cpp

HEADERS += \
    ajustes.h \
    arqueos.h \
    auditoria.h \
    caja.h \
    ciudades.h \
    clientes.h \
    cobros.h \
    cobrotipo.h \
    compras.h \
    cotizaciones.h \
    cuentascobrar.h \
    cuentaspagar.h \
    departamentos.h \
    detalleajustes.h \
    detallecobros.h \
    detallecompras.h \
    detalledevolucionesproveedores.h \
    detalleordenescompra.h \
    detalleordenespago.h \
    detallepedidosclientes.h \
    detallepedidosproveedores.h \
    detallepresupuestos.h \
    detalleventas.h \
    devolucionesproveedores.h \
    empleados.h \
    login.h \
    marcas.h \
    menuprincipal.h \
    monedas.h \
    ordenescompra.h \
    ordenespago.h \
    pagos.h \
    pedidosclientes.h \
    pedidosproveedores.h \
    presupuestos.h \
    productos.h \
    proveedores.h \
    stock.h \
    tipoajustes.h \
    usuarios.h \
    ventas.h \
    ventaspedidos.h

FORMS += \
    ajustes.ui \
    arqueos.ui \
    auditoria.ui \
    caja.ui \
    ciudades.ui \
    clientes.ui \
    cobros.ui \
    cobrotipo.ui \
    compras.ui \
    cotizaciones.ui \
    cuentascobrar.ui \
    cuentaspagar.ui \
    departamentos.ui \
    detalleajustes.ui \
    detallecobros.ui \
    detallecompras.ui \
    detalledevolucionesproveedores.ui \
    detalleordenescompra.ui \
    detalleordenespago.ui \
    detallepedidosclientes.ui \
    detallepedidosproveedores.ui \
    detallepresupuestos.ui \
    detalleventas.ui \
    devolucionesproveedores.ui \
    empleados.ui \
    login.ui \
    marcas.ui \
    menuprincipal.ui \
    monedas.ui \
    ordenescompra.ui \
    ordenespago.ui \
    pagos.ui \
    pedidosclientes.ui \
    pedidosproveedores.ui \
    presupuestos.ui \
    productos.ui \
    proveedores.ui \
    stock.ui \
    tipoajustes.ui \
    usuarios.ui \
    ventas.ui \
    ventaspedidos.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
