/*
 * $Id$
 *
 * Copyright (C) 2011 Steve Tell
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Python bindings for urjtag intially written by Steve Tell.
 * Additional methods by Jonathan Stroud.
 *
 */
#include <Python.h>
#include "structmember.h"
#include "pycompat23.h"

#include <sysdep.h>

#include <urjtag/urjtag.h>
#include <urjtag/chain.h>
#include <urjtag/cmd.h>

static PyObject *UrjtagError;

typedef struct
{
    PyObject_HEAD urj_chain_t *urchain;
} urj_pychain_t;

static void
urj_pyc_dealloc (urj_pychain_t *self)
{
    urj_tap_chain_free (self->urchain);
    Py_TYPE (self)->tp_free ((PyObject *) self);
}

static PyObject *
urj_pyc_new (PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    urj_pychain_t *self;

    self = (urj_pychain_t *) type->tp_alloc (type, 0);
    if (self == NULL)
        return NULL;

    self->urchain = urj_tap_chain_alloc ();
    if (self->urchain == NULL)
    {
        Py_DECREF (self);
        return PyErr_NoMemory ();
    }
    self->urchain->main_part = 0;
    return (PyObject *) self;
}


/* python helpers for methods */

/*
 * propagate return value from liburjtag function that returns
 * URJ_STATUS_OK on success or somthing else on failure.
 * if OK, return python "none".
 * else, throw a python exception
 */
static PyObject *
urj_py_chkret (int rc)
{
    if (rc == URJ_STATUS_OK)
        return Py_BuildValue ("");  /* python "None" */

    if (urj_error_get ())
    {
        PyErr_SetString (UrjtagError, urj_error_describe ());
        urj_error_reset ();
    }
    else
        PyErr_SetString (UrjtagError,
                         _("liburjtag BUG: unknown urjtag error"));
    return NULL;
}

#define UPRC_CBL 1
#define UPRC_DET 2
#define UPRC_BUS 4

/* perform selected prerequesite checks on the state of a chain object
 * returns nonzero on success.
 * if 0 is returned, python exception has been posted and
 *     caller must return NULL to signal python exception.
 */
static int
urj_pyc_precheck (urj_chain_t *urc, int checks_needed)
{
    if (urc == NULL)
    {
        PyErr_SetString (PyExc_RuntimeError, _("liburjtag python binding BUG: null chain"));
        return 0;
    }

    if (checks_needed & UPRC_CBL)
    {
        if (urj_cmd_test_cable (urc) != URJ_STATUS_OK)
        {
            PyErr_SetString (UrjtagError, _("cable() has not been called"));
            return 0;
        }
    }

    if (checks_needed & UPRC_DET)
    {
        if (urc->parts == NULL)
        {
            PyErr_SetString (PyExc_RuntimeError,
                             _("no parts: detect or addpart not called on this chain"));
            return 0;
        }
    }

    if (checks_needed & UPRC_BUS)
    {
        if (!urj_bus)   /* why is this a global and not a chain property? */
        {
            PyErr_SetString (PyExc_RuntimeError,
                             _("Bus missing: initbus not called?"));
            return 0;
        }
        if (!urj_bus->driver)
        {
            PyErr_SetString (PyExc_RuntimeError,
                             _("Bus driver missing: initbus not called?"));
            return 0;
        }
    }

    return 1;
}


/* urj_chain_t / urjtag.chain methods */

static PyObject *
urj_pyc_cable (urj_pychain_t *self, PyObject *args)
{
    char *cable_params[5] = { NULL, NULL, NULL, NULL, NULL };
    urj_chain_t *urc = self->urchain;
    char *drivername;

    if (!urj_pyc_precheck (urc, 0))
        return NULL;

    if (!PyArg_ParseTuple (args, "s|ssss",
                           &drivername,
                           &cable_params[0],
                           &cable_params[1],
                           &cable_params[2], &cable_params[3]))
        return NULL;

    return urj_py_chkret (urj_tap_chain_connect (urc, drivername, cable_params));
}

static PyObject *
urj_pyc_disconnect (urj_pychain_t *self)
{
    urj_chain_t *urc = self->urchain;
    if (!urj_pyc_precheck (urc, 0))
        return NULL;
    urj_tap_chain_disconnect (urc);
    return Py_BuildValue ("");
}

static PyObject *
urj_pyc_test_cable (urj_pychain_t *self)
{
    urj_chain_t *urc = self->urchain;
    if (!urj_pyc_precheck (urc, 0))
        return NULL;
    return urj_py_chkret (urj_cmd_test_cable (urc));
}

static PyObject *
urj_pyc_tap_detect (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    int maxirlen = 0;
    if (!PyArg_ParseTuple (args, "|i", &maxirlen))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;
    return urj_py_chkret (urj_tap_detect (urc, maxirlen));
}

static PyObject *
urj_pyc_len (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    if (!urj_pyc_precheck (urc, UPRC_CBL|UPRC_DET))
        return NULL;

    return Py_BuildValue ("i", urc->parts->len);
}

static PyObject *
urj_pyc_partid (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    int partno;
    if (!PyArg_ParseTuple (args, "i", &partno))
        return NULL;

    if (!urj_pyc_precheck (urc, UPRC_CBL|UPRC_DET))
        return NULL;

    if (partno >= urc->parts->len)
    {
        PyErr_SetString (PyExc_RuntimeError, _("part number out of range"));
        return NULL;
    }
    else
    {
        urj_part_t *p;
        uint32_t id;

        p = urc->parts->parts[partno];
        id = urj_tap_register_get_value (p->id);
        return Py_BuildValue ("i", id);
    }
}

static PyObject *
urj_pyc_reset (urj_pychain_t *self)
{
    urj_chain_t *urc = self->urchain;
    PyObject *rc;

    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    rc = urj_py_chkret (urj_tap_reset_bypass (urc));
    urj_tap_chain_flush (urc);
    return rc;
}

static PyObject *
urj_pyc_set_trst (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    int trstval;
    if (!PyArg_ParseTuple (args, "i", &trstval))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;
    urj_tap_chain_set_trst (urc, trstval);
    return Py_BuildValue ("");
}

static PyObject *
urj_pyc_get_trst (urj_pychain_t *self)
{
    int trstval;
    urj_chain_t *urc = self->urchain;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    trstval = urj_tap_chain_get_trst (urc);
    return Py_BuildValue ("i", trstval);
}

static PyObject *
urj_pyc_set_pod_signal (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    uint32_t mask, val, oldval;
    if (!PyArg_ParseTuple (args, "ii", &mask, &val))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    oldval = urj_tap_chain_set_pod_signal (urc, mask, val);
    return Py_BuildValue ("i", oldval);
}

static PyObject *
urj_pyc_get_pod_signal (urj_pychain_t *self, PyObject *args)
{
    uint32_t sig;
    uint32_t val;
    urj_chain_t *urc = self->urchain;
    if (!PyArg_ParseTuple (args, "i", &sig))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    val = urj_tap_chain_get_pod_signal (urc, sig);
    return Py_BuildValue ("i", val);
}

static PyObject *
urj_pyc_set_frequency (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    uint32_t freq;
    if (!PyArg_ParseTuple (args, "i", &freq))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    urj_tap_cable_set_frequency (urc->cable, freq);
    return Py_BuildValue ("");
}

static PyObject *
urj_pyc_get_frequency (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    unsigned long freq;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    freq = urj_tap_cable_get_frequency (urc->cable);

    return Py_BuildValue ("i", (uint32_t) freq);
}

/* set instruction for the active part
 */
static PyObject *
urj_pyc_set_instruction (urj_pychain_t *self, PyObject *args)
{
    char *instname;
    urj_part_t *part;
    urj_chain_t *urc = self->urchain;
    if (!PyArg_ParseTuple (args, "s", &instname))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    part = urj_tap_chain_active_part (urc);
    if (part == NULL)
    {
        PyErr_SetString (UrjtagError, _("No active part on chain"));
        return NULL;
    }
    urj_part_set_instruction (part, instname);
    return Py_BuildValue ("");
}

static PyObject *
urj_pyc_shift_ir (urj_pychain_t *self)
{
    urj_chain_t *urc = self->urchain;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    return urj_py_chkret (urj_tap_chain_shift_instructions (urc));
}

static PyObject *
urj_pyc_shift_dr (urj_pychain_t *self)
{
    urj_chain_t *urc = self->urchain;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    /*  TODO: need a way to not capture the TDO output
     */
    return urj_py_chkret (urj_tap_chain_shift_data_registers (urc, 1));
}

static PyObject *
urj_pyc_get_dr (urj_pychain_t *self, int in, int string, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    urj_part_t *part;
    urj_tap_register_t *r;
    urj_data_register_t *dr;
    urj_part_instruction_t *active_ir;
    int lsb = -1;
    int msb = -1;

    if (!PyArg_ParseTuple (args, "|ii", &msb, &lsb))
        return NULL;
    if (lsb == -1)
        lsb = msb;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    part = urj_tap_chain_active_part (urc);
    if (part == NULL)
    {
        PyErr_SetString (UrjtagError, _("no active part in chain"));
        return NULL;
    }
    active_ir = part->active_instruction;
    if (active_ir == NULL)
    {
        PyErr_SetString (UrjtagError, _("part without active instruction"));
        return NULL;
    }
    dr = active_ir->data_register;
    if (dr == NULL)
    {
        PyErr_SetString (UrjtagError,
                         _("instruction without active data register"));
        return NULL;
    }

    if (in)
        r = dr->in;             /* input buffer for next shift_dr */
    else
        r = dr->out;            /* recently captured+scanned-out values */

    if (msb == -1)
    {
        if (string)
            return Py_BuildValue ("s", urj_tap_register_get_string (r));
        else
            return Py_BuildValue ("L", urj_tap_register_get_value (r));
    }
    else
    {
        if (string)
            return Py_BuildValue (""); /* TODO urj_tap_register_get_string_bit_range (r, msb, lsb)); */
        else
            return Py_BuildValue ("L", urj_tap_register_get_value_bit_range (r, msb, lsb));
    }
}

static PyObject *
urj_pyc_get_str_dr_out (urj_pychain_t *self, PyObject *args)
{
    return urj_pyc_get_dr (self, 0, 1, args);
}

static PyObject *
urj_pyc_get_str_dr_in (urj_pychain_t *self, PyObject *args)
{
    return urj_pyc_get_dr (self, 1, 1, args);
}

static PyObject *
urj_pyc_get_int_dr_out (urj_pychain_t *self, PyObject *args)
{
    return urj_pyc_get_dr (self, 0, 0, args);
}

static PyObject *
urj_pyc_get_int_dr_in (urj_pychain_t *self, PyObject *args)
{
    return urj_pyc_get_dr (self, 1, 0, args);
}

static PyObject *
urj_pyc_set_dr (urj_pychain_t *self, int in, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    urj_part_t *part;
    urj_tap_register_t *r;
    urj_data_register_t *dr;
    urj_part_instruction_t *active_ir;
    char *newstr = NULL;
    uint64_t newval;
    int lsb = -1;
    int msb = -1;

    if (!PyArg_ParseTuple (args, "s|ii", &newstr, &msb, &lsb))
    {
        PyErr_Clear ();
        if (!PyArg_ParseTuple (args, "L|ii", &newval, &msb, &lsb))
            return NULL;
    }

    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    part = urj_tap_chain_active_part (urc);
    if (part == NULL)
    {
        PyErr_SetString (UrjtagError, _("no active part in chain"));
        return NULL;
    }
    active_ir = part->active_instruction;
    if (active_ir == NULL)
    {
        PyErr_SetString (UrjtagError, _("part without active instruction"));
        return NULL;
    }
    dr = active_ir->data_register;
    if (dr == NULL)
    {
        PyErr_SetString (UrjtagError,
                         _("instruction without active data register"));
        return NULL;
    }

    if (in)
        r = dr->in;
    else
        r = dr->out;

    if (msb == -1)
    {
        if (newstr)
            return urj_py_chkret (urj_tap_register_set_string(r, newstr));
        else
            return urj_py_chkret (urj_tap_register_set_value(r, newval));
    }
    else
    {
        if (lsb == -1)
            lsb = msb;

        if (newstr)
            /* TODO urj_tap_register_set_string_bit_range(r, newstr, msb, lsb); */
            return Py_BuildValue ("");
        else
            return urj_py_chkret (urj_tap_register_set_value_bit_range(r, newval, msb, lsb));
    }
}

static PyObject *
urj_pyc_set_dr_out (urj_pychain_t *self, PyObject *args)
{
    return urj_pyc_set_dr (self, 0, args);
}

static PyObject *
urj_pyc_set_dr_in (urj_pychain_t *self, PyObject *args)
{
    return urj_pyc_set_dr (self, 1, args);
}

static PyObject *
urj_pyc_run_svf (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    char *fname;
    int stop = 0;
    unsigned long ref_freq = 0;
    FILE *svf_file;
    PyObject *rc;

    if (!PyArg_ParseTuple (args, "s|iI", &fname, &stop, &ref_freq))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    svf_file = fopen (fname, FOPEN_R);
    if (!svf_file)
    {
        PyErr_SetFromErrnoWithFilename(PyExc_IOError, fname);
        return NULL;
    }
    rc = urj_py_chkret (urj_svf_run (urc, svf_file, stop, ref_freq));
    fclose (svf_file);
    return rc;
}

static PyObject *
urj_pyc_addpart (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    long unsigned len;

    if (!PyArg_ParseTuple (args, "i", &len))
        return NULL;

    if (!urj_pyc_precheck (urc, UPRC_CBL))
        return NULL;

    if (urj_tap_manual_add (urc, len) < 0)
    {
            PyErr_SetString (PyExc_RuntimeError,
                             _("urj_tap_manual_add failed"));
            return NULL;
    }

    if (urc->parts == NULL)
    {
        PyErr_SetString (PyExc_RuntimeError,
                         _("addpart: internal error; no parts."));
        return NULL;
    }

    // @@@@ RFHH this cannot be
    if (urc->parts->len == 0)
    {
        urj_part_parts_free (urc->parts);
        self->urchain->parts = NULL;
        PyErr_SetString (PyExc_RuntimeError,
                         _("addpart: internal error; parts->len==0."));
        return NULL;
    }

    urj_part_parts_set_instruction (urc->parts, "BYPASS");
    urj_tap_chain_shift_instructions (urc);
    return Py_BuildValue ("");
}

static PyObject *
urj_pyc_add_register (urj_pychain_t *self, PyObject *args)
{
    char *regname;
    int reglen;
    urj_part_t *part;
    urj_chain_t *urc = self->urchain;

    if (!PyArg_ParseTuple (args, "si", &regname, &reglen))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL|UPRC_DET))
        return NULL;

    part = urj_tap_chain_active_part (urc);
    if (part == NULL)
    {
        if (urj_error_get ())
        {
            PyErr_SetString (UrjtagError, urj_error_describe ());
            urj_error_reset ();
        }
        else
            PyErr_SetString (UrjtagError,
                             _("liburjtag BUG: unknown urjtag error"));
        return NULL;
    }

    return urj_py_chkret (urj_part_data_register_define (part, regname, reglen));
}

static PyObject *
urj_pyc_add_instruction (urj_pychain_t *self, PyObject *args)
{
    char *instname;
    char *code;
    char *regname;
    urj_part_t *part;
    urj_chain_t *urc = self->urchain;

    if (!PyArg_ParseTuple (args, "sss", &instname, &code, &regname))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL|UPRC_DET))
        return NULL;
    part = urj_tap_chain_active_part (urc);

    if (part == NULL)
    {
        if (urj_error_get ())
        {
            PyErr_SetString (UrjtagError, urj_error_describe ());
            urj_error_reset ();
        }
        else
            PyErr_SetString (UrjtagError,
                             _("liburjtag BUG: unknown urjtag error"));
        return NULL;
    }

    if (urj_part_instruction_define (part, instname, code, regname) == NULL)
        return urj_py_chkret (URJ_STATUS_FAIL);
    else
        return Py_BuildValue ("");
}

static PyObject *
urj_pyc_setpart (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    int part;
    if (!PyArg_ParseTuple (args, "i", &part))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL|UPRC_DET))
        return NULL;

    urc->active_part = part;
    return Py_BuildValue ("");
}

static PyObject *
urj_pyc_initbus (urj_pychain_t *self, PyObject *args)
{
    char *bus_params[5] = { NULL, NULL, NULL, NULL, NULL };
    char *drivername;
    urj_chain_t *urc = self->urchain;

    if (!PyArg_ParseTuple (args, "s|ssss",
                           &drivername,
                           &bus_params[0], &bus_params[1], &bus_params[2],
                           &bus_params[3]))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL|UPRC_DET))
        return NULL;

    return urj_py_chkret (urj_bus_init (urc, drivername, bus_params));
}

static PyObject *
urj_pyc_detectflash (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    int adr;
    if (!PyArg_ParseTuple (args, "i", &adr))
        return NULL;
    if (!urj_pyc_precheck (urc, UPRC_CBL|UPRC_BUS))
        return NULL;

    return Py_BuildValue ("i",
                          urj_flash_detectflash (URJ_LOG_LEVEL_NORMAL,
                                                 urj_bus, adr));
}

static PyObject *
urj_pyc_peek (urj_pychain_t *self, PyObject *args)
{
    long unsigned adr;
    uint32_t val;
    urj_bus_area_t area;
    urj_chain_t *urc = self->urchain;

    if (!PyArg_ParseTuple (args, "i", &adr))
        return NULL;

    if (!urj_pyc_precheck (urc, UPRC_CBL|UPRC_BUS))
        return NULL;

    URJ_BUS_PREPARE (urj_bus);
    URJ_BUS_AREA (urj_bus, adr, &area);
    val = URJ_BUS_READ (urj_bus, adr);

    switch (area.width)
    {
    case 8:
        val &= 0xff;
        break;
    case 16:
        val &= 0xffff;
        break;
    default:
        break;
    }
    return Py_BuildValue ("i", val);
}

static PyObject *
urj_pyc_poke (urj_pychain_t *self, PyObject *args)
{
    long unsigned adr, val;
    urj_bus_area_t area;
    urj_chain_t *urc = self->urchain;

    if (!PyArg_ParseTuple (args, "ii", &adr, &val))
        return NULL;

    if (!urj_pyc_precheck (urc, UPRC_CBL|UPRC_BUS))
        return NULL;

    URJ_BUS_PREPARE (urj_bus);
    URJ_BUS_AREA (urj_bus, adr, &area);
    URJ_BUS_WRITE (urj_bus, adr, val);
    return Py_BuildValue ("");
}

static PyObject *
urj_pyc_flashmem (urj_pychain_t *self, PyObject *args)
{
    urj_chain_t *urc = self->urchain;
    int msbin;
    int noverify = 0;
    long unsigned adr = 0;
    FILE *f;
    char *optstr = NULL;
    char *fname = NULL;
    int r;

    if (!urj_pyc_precheck (urc, UPRC_CBL|UPRC_BUS))
        return NULL;

    if (!PyArg_ParseTuple
        (args, "ss|i", &optstr, &fname, &noverify))
        return NULL;

    msbin = strcasecmp ("msbin", optstr) == 0;
    if (!msbin && urj_cmd_get_number (optstr, &adr) != URJ_STATUS_OK)
        return NULL;

    f = fopen (fname, FOPEN_R);
    if (!f)
    {
        PyErr_SetFromErrnoWithFilename(PyExc_IOError, fname);
        return NULL;
    }

    if (msbin)
        r = urj_flashmsbin (urj_bus, f, noverify);
    else
        r = urj_flashmem (urj_bus, f, adr, noverify);

    fclose (f);
    return Py_BuildValue ("i", r);
}

static PyMethodDef urj_pyc_methods[] =
{
    {"cable", (PyCFunction) urj_pyc_cable, METH_VARARGS,
     "Connect to the jtag hardware cable of the specified name and type."},
    {"test_cable", (PyCFunction) urj_pyc_test_cable, METH_NOARGS,
     "check that the jtag cable is connected to a valid chain"},
    {"disconnect", (PyCFunction) urj_pyc_disconnect, METH_NOARGS,
     "Disconnect from the jtag hardware cable"},
    {"tap_detect", (PyCFunction) urj_pyc_tap_detect, METH_VARARGS,
     "Identify the chips on the chain"},
    {"len", (PyCFunction) urj_pyc_len, METH_NOARGS,
     "Return the length of the TAP chain"},
    {"reset", (PyCFunction) urj_pyc_reset, METH_NOARGS,
     "Perform jtag reset using TMS"},
    {"partid", (PyCFunction) urj_pyc_partid, METH_VARARGS,
     "Return the IDCODE for the indicated part number in the chain"},
    {"set_trst", (PyCFunction) urj_pyc_set_trst, METH_VARARGS,
     "set the TRST output of the cable"},
    {"get_trst", (PyCFunction) urj_pyc_get_trst, METH_NOARGS,
     "get the current value of the TRST output of the cable"},
    {"set_pod_signal", (PyCFunction) urj_pyc_set_pod_signal, METH_VARARGS,
     "set an auxiliary pod signal"},
    {"get_pod_signal", (PyCFunction) urj_pyc_get_pod_signal, METH_VARARGS,
     "get the current value of an auxiliary pod signal"},
    {"set_frequency", (PyCFunction) urj_pyc_set_frequency, METH_VARARGS,
     "Change the TCK frequency to be at most the specified value in Hz"},
    {"get_frequency", (PyCFunction) urj_pyc_get_frequency, METH_NOARGS,
     "get the current TCK frequency"},
    {"set_instruction", (PyCFunction) urj_pyc_set_instruction, METH_VARARGS,
     "Set values in the instruction register holding buffer"},
    {"shift_ir", (PyCFunction) urj_pyc_shift_ir, METH_NOARGS,
     "scan values through the instruction register"},
    {"shift_dr", (PyCFunction) urj_pyc_shift_dr, METH_NOARGS,
     "scan values through the data register"},

    {"get_dr_in_string", (PyCFunction) urj_pyc_get_str_dr_in, METH_VARARGS,
     "get bits that will be scanned in on next shift_dr, as string"},
    {"get_dr_out_string", (PyCFunction) urj_pyc_get_str_dr_out, METH_VARARGS,
     "retrieve values scanned out from the data registers on the last shift_dr, as string"},
    {"get_dr_in", (PyCFunction) urj_pyc_get_int_dr_in, METH_VARARGS,
     "get bits that will be scanned in on next shift_dr, as integer"},
    {"get_dr_out", (PyCFunction) urj_pyc_get_int_dr_out, METH_VARARGS,
     "retrieve values scanned out from the data registers on the last shift_dr, as integer"},

    {"set_dr_in", (PyCFunction) urj_pyc_set_dr_in, METH_VARARGS,
     "set bits that will be scanned in on next shiftdr"},
    {"set_dr_out", (PyCFunction) urj_pyc_set_dr_out, METH_VARARGS,
     "set the holding register for values scanned out from the data registers"},
    {"run_svf", (PyCFunction) urj_pyc_run_svf, METH_VARARGS,
     "Play a named SVF file; optionally setting stop-on-mismatch and runtest frequency"},
    {"addpart", (PyCFunction) urj_pyc_addpart, METH_VARARGS,
     "manually adds parts on the JTAG chain"},
    {"add_instruction", (PyCFunction) urj_pyc_add_instruction, METH_VARARGS,
     "manually add intruction to the current part"},
    {"add_register", (PyCFunction) urj_pyc_add_register, METH_VARARGS,
     "manually add register to current part on the JTAG chain"},
    {"part", (PyCFunction) urj_pyc_setpart, METH_VARARGS,
     "change active part for current JTAG chain"},
    {"initbus", (PyCFunction) urj_pyc_initbus, METH_VARARGS,
     "initialize bus driver for active part"},
    {"detectflash", (PyCFunction) urj_pyc_detectflash, METH_VARARGS,
     "Detect parameters of flash chips attached to a part"},
    {"peek", (PyCFunction) urj_pyc_peek, METH_VARARGS,
     "read a single word"},
    {"poke", (PyCFunction) urj_pyc_poke, METH_VARARGS,
     "write a single word"},
    {"flashmem", (PyCFunction) urj_pyc_flashmem, METH_VARARGS,
     "burn flash memory with data from a file"},
    {NULL}                      /* Sentinel */
};

static PyTypeObject urj_pychain_Type =
{
    PyVarObject_HEAD_INIT (NULL, 0) "urjtag.chain", /* tp_name */
    sizeof (urj_pychain_t),             /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor) urj_pyc_dealloc, /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_compare */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "JTAG chain objects",       /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    urj_pyc_methods,              /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    urj_pyc_new,                  /* tp_new */
};

/************************************************************************
 * module methods that are not part of any type
 */

static PyObject *
urjtag_loglevel (PyObject *self, PyObject *args)
{
    int loglevel; /* TODO: accept string or symbol and map to the enum */
    if (!PyArg_ParseTuple (args, "i", &loglevel))
        return NULL;
    urj_log_state.level = loglevel;
    return Py_BuildValue ("");
}

static PyMethodDef module_methods[] =
{
    {"loglevel", urjtag_loglevel, METH_VARARGS,
     "Set log level of the urjtag library"},
    {NULL}                      /* Sentinel */
};

static struct PyModuleDef chain_moduledef =
{
    PyModuleDef_HEAD_INIT,
    "urjtag",
    "Python extension module for urjtag",
    -1,
    module_methods,
};

MODINIT_DECL (urjtag)
{
    PyObject *m;

    if (PyType_Ready (&urj_pychain_Type) < 0)
        return MODINIT_ERROR_VAL;

    m = PyModule_Create (&chain_moduledef);

    if (m == NULL)
        return MODINIT_ERROR_VAL;

    UrjtagError = PyErr_NewException ("urjtag.error", NULL, NULL);
    Py_INCREF (UrjtagError);
    PyModule_AddObject (m, "error", UrjtagError);

    PyModule_AddIntMacro(m, URJ_LOG_LEVEL_ALL     );
    PyModule_AddIntMacro(m, URJ_LOG_LEVEL_COMM    );
    PyModule_AddIntMacro(m, URJ_LOG_LEVEL_DEBUG   );
    PyModule_AddIntMacro(m, URJ_LOG_LEVEL_DETAIL  );
    PyModule_AddIntMacro(m, URJ_LOG_LEVEL_NORMAL  );
    PyModule_AddIntMacro(m, URJ_LOG_LEVEL_WARNING );
    PyModule_AddIntMacro(m, URJ_LOG_LEVEL_ERROR   );
    PyModule_AddIntMacro(m, URJ_LOG_LEVEL_SILENT  );

    PyModule_AddIntMacro(m, URJ_POD_CS_TDI    );
    PyModule_AddIntMacro(m, URJ_POD_CS_TCK    );
    PyModule_AddIntMacro(m, URJ_POD_CS_TMS    );
    PyModule_AddIntMacro(m, URJ_POD_CS_TRST   );
    PyModule_AddIntMacro(m, URJ_POD_CS_RESET  );
    PyModule_AddIntMacro(m, URJ_POD_CS_SCK    );
    PyModule_AddIntMacro(m, URJ_POD_CS_SDA    );
    PyModule_AddIntMacro(m, URJ_POD_CS_SS     );

    Py_INCREF (&urj_pychain_Type);
    PyModule_AddObject (m, "chain", (PyObject *) &urj_pychain_Type);

    return MODINIT_SUCCESS_VAL (m);
}

/* Local Variables: */
/* mode:c */
/* comment-column:0 */
/* c-basic-offset:4 */
/* space-before-funcall:t */
/* indent-tabs-mode:nil */
/* End: */
