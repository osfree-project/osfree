
#ifndef ___gio_marshal_MARSHAL_H__
#define ___gio_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:STRING,STRING,STRING,FLAGS (./gio-marshal.list:1) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_STRING_STRING_FLAGS (GClosure     *closure,
                                                                    GValue       *return_value,
                                                                    guint         n_param_values,
                                                                    const GValue *param_values,
                                                                    gpointer      invocation_hint,
                                                                    gpointer      marshal_data);

/* VOID:STRING,BOXED (./gio-marshal.list:2) */
G_GNUC_INTERNAL void _gio_marshal_VOID__STRING_BOXED (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);

/* VOID:BOOLEAN,POINTER (./gio-marshal.list:3) */
G_GNUC_INTERNAL void _gio_marshal_VOID__BOOLEAN_POINTER (GClosure     *closure,
                                                         GValue       *return_value,
                                                         guint         n_param_values,
                                                         const GValue *param_values,
                                                         gpointer      invocation_hint,
                                                         gpointer      marshal_data);

/* VOID:OBJECT,OBJECT,ENUM (./gio-marshal.list:4) */
G_GNUC_INTERNAL void _gio_marshal_VOID__OBJECT_OBJECT_ENUM (GClosure     *closure,
                                                            GValue       *return_value,
                                                            guint         n_param_values,
                                                            const GValue *param_values,
                                                            gpointer      invocation_hint,
                                                            gpointer      marshal_data);

G_END_DECLS

#endif /* ___gio_marshal_MARSHAL_H__ */

