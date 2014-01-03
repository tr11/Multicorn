/*-------------------------------------------------------------------------
 *
 * The Multicorn Foreign Data Wrapper allows you to fetch foreign data in
 * Python in your PostgreSQL.
 *
 * This module contains helpers meant to be called from python code.
 *
 * This software is released under the postgresql licence
 *
 * author: Kozea
 *
 *
 *-------------------------------------------------------------------------
 */
#include <Python.h>
#include "postgres.h"


struct module_state
{
	PyObject   *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

static PyObject *
log_to_postgres(PyObject *self, PyObject *args, PyObject *kwargs)
{
	char	   *message = NULL;
	char	   *hintstr = NULL,
			   *detailstr = NULL;
	int			level = 1;
	int			severity;
	PyObject   *hint,
			   *p_message,
			   *detail;

	if (!PyArg_ParseTuple(args, "O|i", &p_message, &level))
	{
		errorCheck();
		Py_INCREF(Py_None);
		return Py_None;
	}
	if (PyBytes_Check(p_message))
	{
		message = PyBytes_AsString(p_message);
	}
	else if (PyUnicode_Check(p_message))
	{
		message = strdup(PyUnicode_AsPgString(p_message));
	}
	else
	{

		PyObject   *temp = PyObject_Str(p_message);

		errorCheck();
		message = strdup(PyString_AsString(temp));
		errorCheck();
		Py_DECREF(temp);
	}
	switch (level)
	{
		case 0:
			severity = DEBUG1;
			break;
		case 1:
			severity = NOTICE;
			break;
		case 2:
			severity = WARNING;
			break;
		case 3:
			severity = ERROR;
			break;
		case 4:
			severity = FATAL;
			break;
		default:
			severity = INFO;
			break;
	}
	hint = PyDict_GetItemString(kwargs, "hint");
	detail = PyDict_GetItemString(kwargs, "detail");
	if (errstart(severity, __FILE__, __LINE__, PG_FUNCNAME_MACRO, TEXTDOMAIN))
	{
		errmsg("%s", message);
		if (hint != NULL && hint != Py_None)
		{
			hintstr = PyString_AsString(hint);
			errhint("%s", hintstr);
			Py_DECREF(hint);
		}
		if (detail != NULL && detail != Py_None)
		{
			detailstr = PyString_AsString(detail);
			errdetail("%s", detailstr);
			Py_DECREF(detail);
		}
		Py_DECREF(args);
		Py_DECREF(kwargs);
		Py_INCREF(Py_None);
		errfinish(0);
	}
	else
	{
		Py_DECREF(args);
		Py_DECREF(kwargs);
		Py_INCREF(Py_None);
	}
	return Py_None;
}

static PyMethodDef UtilsMethods[] = {
	{"_log_to_postgres", (PyCFunction) log_to_postgres, METH_VARARGS | METH_KEYWORDS, "Log to postresql client"},
	{NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"multicorn._utils",
	NULL,
	sizeof(struct module_state),
	UtilsMethods,
	NULL,
	NULL,
	NULL,
	NULL
};

#define INITERROR return NULL

PyObject *
PyInit__utils(void)
#else
#define INITERROR return

void
init_utils(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
	PyObject   *module = PyModule_Create(&moduledef);
#else
	PyObject   *module = Py_InitModule("multicorn._utils", UtilsMethods);
#endif

	if (module == NULL)
		INITERROR;
	struct module_state *st = GETSTATE(module);

#if PY_MAJOR_VERSION >= 3
	return module;
#endif
}
