-- =============================================
-- TABLAS MAESTRAS BÁSICAS
-- =============================================

CREATE TABLE moneda (
    id SERIAL PRIMARY KEY,
    codigo VARCHAR(3) UNIQUE NOT NULL, -- Gs, USD, BRL, ARS
    nombre VARCHAR(50) NOT NULL,
    simbolo VARCHAR(5),
    cambio NUMERIC(15,2) DEFAULT 1,
    activo BOOLEAN DEFAULT true
);

CREATE TABLE departamento (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL
);

CREATE TABLE ciudad (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL,
    codigo_postal VARCHAR(10),
    departamento_id INTEGER REFERENCES departamento(id) NOT NULL
);

CREATE TABLE marca (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL,
    descripcion TEXT,
    activo BOOLEAN DEFAULT true
);

CREATE TABLE proveedor (
    id SERIAL PRIMARY KEY,
    ruc VARCHAR(20) UNIQUE NOT NULL,
    razon_social VARCHAR(255) NOT NULL,
    nombre_comercial VARCHAR(255),
    direccion TEXT,
    telefono VARCHAR(20),
    email VARCHAR(100),
    ciudad_id INTEGER REFERENCES ciudad(id),
    activo BOOLEAN DEFAULT true,
    fecha_registro TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE cliente (
    id SERIAL PRIMARY KEY,
    ruc_ci VARCHAR(20) NOT NULL,
    nombre_razon_social VARCHAR(255) NOT NULL,
    nombre_comercial VARCHAR(255),
    direccion TEXT,
    telefono VARCHAR(20),
    email VARCHAR(100),
    ciudad_id INTEGER REFERENCES ciudad(id),
    limite_credito NUMERIC(15,2) DEFAULT 0,
    activo BOOLEAN DEFAULT true,
    fecha_registro TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE producto (
    id SERIAL PRIMARY KEY,
    codigo VARCHAR(50) UNIQUE NOT NULL,
    nombre VARCHAR(255) NOT NULL,
    descripcion TEXT,
    precio_venta NUMERIC(15,2) NOT NULL,
    precio_compra NUMERIC(15,2) NOT NULL,
    marca_id INTEGER REFERENCES marca(id),
    proveedor_id INTEGER REFERENCES proveedor(id),
    stock_minimo INTEGER DEFAULT 0,
    activo BOOLEAN DEFAULT true,
    iva NUMERIC(3,2) DEFAULT 0.10, -- IVA Paraguay 10%
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- =============================================
-- TABLAS DE USUARIOS Y EMPLEADOS
-- =============================================

CREATE TABLE empleado (
    id SERIAL PRIMARY KEY,
    cedula VARCHAR(20) UNIQUE NOT NULL,
    nombre VARCHAR(100) NOT NULL,
    apellido VARCHAR(100) NOT NULL,
    direccion TEXT,
    telefono VARCHAR(20),
    email VARCHAR(100),
    fecha_nacimiento DATE,
    fecha_contratacion DATE,
    salario NUMERIC(15,2),
    departamento_id INTEGER REFERENCES departamento(id),
    activo BOOLEAN DEFAULT true
);

CREATE TABLE usuario (
    id SERIAL PRIMARY KEY,
    empleado_id INTEGER REFERENCES empleado(id),
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    rol VARCHAR(50) NOT NULL,
    activo BOOLEAN DEFAULT true,
    fecha_creacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ultimo_login TIMESTAMP
);

-- =============================================
-- TABLAS DE CAJA Y COBROS
-- =============================================

CREATE TABLE caja (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL,
    descripcion TEXT,
    saldo_inicial NUMERIC(15,2) DEFAULT 0,
    activa BOOLEAN DEFAULT true
);

CREATE TABLE cobro_tipo (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL,
    descripcion TEXT
);

CREATE TABLE cobro (
    id SERIAL PRIMARY KEY,
    caja_id INTEGER REFERENCES caja(id),
    cliente_id INTEGER REFERENCES cliente(id),
    monto_total NUMERIC(15,2) NOT NULL,
    moneda_id INTEGER REFERENCES moneda(id),
    fecha_cobro TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    observaciones TEXT,
    usuario_id INTEGER REFERENCES usuario(id)
);

CREATE TABLE cobro_detalle (
    id SERIAL PRIMARY KEY,
    cobro_id INTEGER REFERENCES cobro(id),
    cobro_tipo_id INTEGER REFERENCES cobro_tipo(id),
    monto NUMERIC(15,2) NOT NULL,
    referencia VARCHAR(100),
    banco VARCHAR(100),
    nro_cheque VARCHAR(100)
);

CREATE TABLE arqueo (
    id SERIAL PRIMARY KEY,
    caja_id INTEGER REFERENCES caja(id),
    usuario_id INTEGER REFERENCES usuario(id),
    fecha_apertura TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    fecha_cierre TIMESTAMP,
    saldo_inicial NUMERIC(15,2) NOT NULL,
    saldo_final NUMERIC(15,2),
    estado VARCHAR(20) DEFAULT 'ABIERTO'
);

-- =============================================
-- TABLAS DE VENTAS Y PEDIDOS DE CLIENTES
-- =============================================

CREATE TABLE pedido_cliente (
    id SERIAL PRIMARY KEY,
    cliente_id INTEGER REFERENCES cliente(id),
    fecha_pedido TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    fecha_entrega_estimada DATE,
    estado VARCHAR(20) DEFAULT 'PENDIENTE',
    total NUMERIC(15,2) NOT NULL,
    observaciones TEXT,
    usuario_id INTEGER REFERENCES usuario(id)
);

CREATE TABLE pedido_cliente_detalle (
    id SERIAL PRIMARY KEY,
    pedido_cliente_id INTEGER REFERENCES pedido_cliente(id),
    producto_id INTEGER REFERENCES producto(id),
    cantidad INTEGER NOT NULL,
    precio_unitario NUMERIC(15,2) NOT NULL,
    subtotal NUMERIC(15,2) NOT NULL
);

CREATE TABLE venta (
    id SERIAL PRIMARY KEY,
    cliente_id INTEGER REFERENCES cliente(id),
    fecha_venta TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    total NUMERIC(15,2) NOT NULL,
    iva_total NUMERIC(15,2) NOT NULL,
    estado VARCHAR(20) DEFAULT 'PENDIENTE',
    moneda_id INTEGER REFERENCES moneda(id),
    nro_factura VARCHAR(50),
    timbrado VARCHAR(50),
    usuario_id INTEGER REFERENCES usuario(id)
);

CREATE TABLE venta_detalle (
    id SERIAL PRIMARY KEY,
    venta_id INTEGER REFERENCES venta(id),
    producto_id INTEGER REFERENCES producto(id),
    cantidad INTEGER NOT NULL,
    precio_unitario NUMERIC(15,2) NOT NULL,
    iva NUMERIC(15,2) NOT NULL,
    subtotal NUMERIC(15,2) NOT NULL
);

CREATE TABLE pedido_cliente_has_venta (
    id SERIAL PRIMARY KEY,
    pedido_cliente_id INTEGER REFERENCES pedido_cliente(id),
    venta_id INTEGER REFERENCES venta(id),
    fecha_asociacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- =============================================
-- TABLAS DE PRESUPUESTOS Y COTIZACIONES
-- =============================================

CREATE TABLE presupuesto (
    id SERIAL PRIMARY KEY,
    cliente_id INTEGER REFERENCES cliente(id),
    fecha_presupuesto TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    valido_hasta DATE,
    total NUMERIC(15,2) NOT NULL,
    estado VARCHAR(20) DEFAULT 'PENDIENTE',
    observaciones TEXT,
    usuario_id INTEGER REFERENCES usuario(id)
);

CREATE TABLE presupuesto_detalle (
    id SERIAL PRIMARY KEY,
    presupuesto_id INTEGER REFERENCES presupuesto(id),
    producto_id INTEGER REFERENCES producto(id),
    cantidad INTEGER NOT NULL,
    precio_unitario NUMERIC(15,2) NOT NULL,
    subtotal NUMERIC(15,2) NOT NULL
);

CREATE TABLE cotizacion (
    id SERIAL PRIMARY KEY,
    moneda_id INTEGER REFERENCES moneda(id),
    cotizacion_compra NUMERIC(15,2) NOT NULL,
    cotizacion_venta NUMERIC(15,2) NOT NULL,
    fecha_cotizacion DATE NOT NULL,
    usuario_id INTEGER REFERENCES usuario(id)
);

-- =============================================
-- TABLAS DE COMPRAS Y PEDIDOS A PROVEEDORES
-- =============================================

CREATE TABLE pedido_a_proveedor (
    id SERIAL PRIMARY KEY,
    proveedor_id INTEGER REFERENCES proveedor(id),
    fecha_pedido TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    fecha_entrega_estimada DATE,
    estado VARCHAR(20) DEFAULT 'PENDIENTE',
    total NUMERIC(15,2) NOT NULL,
    observaciones TEXT,
    usuario_id INTEGER REFERENCES usuario(id)
);

CREATE TABLE pedido_a_proveedor_detalle (
    id SERIAL PRIMARY KEY,
    pedido_a_proveedor_id INTEGER REFERENCES pedido_a_proveedor(id),
    producto_id INTEGER REFERENCES producto(id),
    cantidad INTEGER NOT NULL,
    precio_unitario NUMERIC(15,2) NOT NULL,
    subtotal NUMERIC(15,2) NOT NULL
);

CREATE TABLE orden_de_compra (
    id SERIAL PRIMARY KEY,
    proveedor_id INTEGER REFERENCES proveedor(id),
    fecha_orden TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    total NUMERIC(15,2) NOT NULL,
    estado VARCHAR(20) DEFAULT 'PENDIENTE',
    observaciones TEXT,
    usuario_id INTEGER REFERENCES usuario(id)
);

CREATE TABLE orden_de_compra_detalle (
    id SERIAL PRIMARY KEY,
    orden_de_compra_id INTEGER REFERENCES orden_de_compra(id),
    producto_id INTEGER REFERENCES producto(id),
    cantidad INTEGER NOT NULL,
    precio_unitario NUMERIC(15,2) NOT NULL,
    subtotal NUMERIC(15,2) NOT NULL
);

CREATE TABLE compra (
    id SERIAL PRIMARY KEY,
    proveedor_id INTEGER REFERENCES proveedor(id),
    fecha_compra TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    total NUMERIC(15,2) NOT NULL,
    iva_total NUMERIC(15,2) NOT NULL,
    estado VARCHAR(20) DEFAULT 'PENDIENTE',
    nro_factura VARCHAR(50),
    timbrado VARCHAR(50),
    usuario_id INTEGER REFERENCES usuario(id)
);

CREATE TABLE compra_detalle (
    id SERIAL PRIMARY KEY,
    compra_id INTEGER REFERENCES compra(id),
    producto_id INTEGER REFERENCES producto(id),
    cantidad INTEGER NOT NULL,
    precio_unitario NUMERIC(15,2) NOT NULL,
    iva NUMERIC(15,2) NOT NULL,
    subtotal NUMERIC(15,2) NOT NULL
);

-- =============================================
-- TABLAS DE PAGOS Y ÓRDENES DE PAGO
-- =============================================

CREATE TABLE orden_de_pago (
    id SERIAL PRIMARY KEY,
    proveedor_id INTEGER REFERENCES proveedor(id),
    fecha_orden TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    total NUMERIC(15,2) NOT NULL,
    estado VARCHAR(20) DEFAULT 'PENDIENTE',
    observaciones TEXT,
    usuario_id INTEGER REFERENCES usuario(id)
);

CREATE TABLE orden_de_pago_detalle (
    id SERIAL PRIMARY KEY,
    orden_de_pago_id INTEGER REFERENCES orden_de_pago(id),
    compra_id INTEGER REFERENCES compra(id),
    monto NUMERIC(15,2) NOT NULL,
    saldo_anterior NUMERIC(15,2) NOT NULL,
    saldo_actual NUMERIC(15,2) NOT NULL
);

CREATE TABLE pago (
    id SERIAL PRIMARY KEY,
    proveedor_id INTEGER REFERENCES proveedor(id),
    monto_total NUMERIC(15,2) NOT NULL,
    fecha_pago TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    moneda_id INTEGER REFERENCES moneda(id),
    observaciones TEXT,
    usuario_id INTEGER REFERENCES usuario(id)
);

-- =============================================
-- TABLAS DE CUENTAS POR COBRAR/PAGAR
-- =============================================

CREATE TABLE cuenta_a_cobrar (
    id SERIAL PRIMARY KEY,
    cliente_id INTEGER REFERENCES cliente(id),
    venta_id INTEGER REFERENCES venta(id),
    monto_total NUMERIC(15,2) NOT NULL,
    saldo_pendiente NUMERIC(15,2) NOT NULL,
    fecha_emision TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    fecha_vencimiento DATE,
    estado VARCHAR(20) DEFAULT 'PENDIENTE'
);

CREATE TABLE cuenta_a_pagar (
    id SERIAL PRIMARY KEY,
    proveedor_id INTEGER REFERENCES proveedor(id),
    compra_id INTEGER REFERENCES compra(id),
    monto_total NUMERIC(15,2) NOT NULL,
    saldo_pendiente NUMERIC(15,2) NOT NULL,
    fecha_emision TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    fecha_vencimiento DATE,
    estado VARCHAR(20) DEFAULT 'PENDIENTE'
);

-- =============================================
-- TABLAS DE STOCK Y DEVOLUCIONES
-- =============================================

CREATE TABLE stock (
    id SERIAL PRIMARY KEY,
    producto_id INTEGER REFERENCES producto(id),
    cantidad INTEGER NOT NULL,
    cantidad_minima INTEGER DEFAULT 0,
    cantidad_maxima INTEGER DEFAULT 0,
    ubicacion VARCHAR(100),
    fecha_actualizacion TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE ajustes_tipo (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL,
    descripcion TEXT,
    afecta_stock BOOLEAN DEFAULT true
);

CREATE TABLE ajustes (
    id SERIAL PRIMARY KEY,
    ajuste_tipo_id INTEGER REFERENCES ajustes_tipo(id),
    fecha_ajuste TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    observaciones TEXT,
    usuario_id INTEGER REFERENCES usuario(id)
);

CREATE TABLE ajuste_detalle (
    id SERIAL PRIMARY KEY,
    ajuste_id INTEGER REFERENCES ajustes(id),
    producto_id INTEGER REFERENCES producto(id),
    cantidad_anterior INTEGER NOT NULL,
    cantidad_nueva INTEGER NOT NULL,
    diferencia INTEGER NOT NULL,
    motivo TEXT
);

CREATE TABLE devolucion_a_proveedor (
    id SERIAL PRIMARY KEY,
    proveedor_id INTEGER REFERENCES proveedor(id),
    compra_id INTEGER REFERENCES compra(id),
    fecha_devolucion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    total NUMERIC(15,2) NOT NULL,
    observaciones TEXT,
    usuario_id INTEGER REFERENCES usuario(id)
);

CREATE TABLE devolucion_proveedor_detalle (
    id SERIAL PRIMARY KEY,
    devolucion_a_proveedor_id INTEGER REFERENCES devolucion_a_proveedor(id),
    producto_id INTEGER REFERENCES producto(id),
    cantidad INTEGER NOT NULL,
    precio_unitario NUMERIC(15,2) NOT NULL,
    subtotal NUMERIC(15,2) NOT NULL
);

-- =============================================
-- TABLA DE AUDITORÍA 
-- =============================================

CREATE TABLE auditoria (
    id SERIAL PRIMARY KEY,
    tabla_afectada VARCHAR(100) NOT NULL,
    registro_id INTEGER NOT NULL,
    accion VARCHAR(50) NOT NULL,
    valor_anterior TEXT,  
    valor_nuevo TEXT,     
    usuario_id INTEGER REFERENCES usuario(id),
    fecha_accion TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip_address VARCHAR(45)
);

-- =============================================
-- ÍNDICES PARA MEJOR PERFORMANCE
-- =============================================

CREATE INDEX idx_cliente_ruc ON cliente(ruc_ci);
CREATE INDEX idx_proveedor_ruc ON proveedor(ruc);
CREATE INDEX idx_producto_codigo ON producto(codigo);
CREATE INDEX idx_venta_fecha ON venta(fecha_venta);
CREATE INDEX idx_compra_fecha ON compra(fecha_compra);
CREATE INDEX idx_cuenta_cobrar_estado ON cuenta_a_cobrar(estado);
CREATE INDEX idx_cuenta_pagar_estado ON cuenta_a_pagar(estado);
CREATE INDEX idx_stock_producto ON stock(producto_id);