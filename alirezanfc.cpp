#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdint.h>
#include "gpio.h"
#include <Python.h>
#include <iostream>
#include <string>
#include <sstream>

#include "nfcdev.h"

using namespace std;

extern "C" {
    void initalirezanfc(void);
}

struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif


//-----------------------------------------------------
static PyObject*
alirezanfc_init(PyObject* self, PyObject* args)
{
   int x = 1;
   pn532init() ;
   if(x){//check if init nfc module return true otherwise return false

          return Py_BuildValue("i", 1);
   }

   return Py_BuildValue("i", 0);
}
//-----------------------------------------------------
static PyObject*
alirezanfc_polling(PyObject* self, PyObject* args)
{
   int x = 1;
   x = poll_mifare() ;
   if(x == 1){//check if polling nfc module return true otherwise return false

          return Py_BuildValue("i", 1);
   }

   return Py_BuildValue("i", 0);
}
//-----------------------------------------------------
static PyObject*
alirezanfc_format_card(PyObject* self, PyObject* args)
{
   if( formatcard() ){

          return Py_BuildValue("i", 1);
   }

   return Py_BuildValue("i", 0);
}
//-----------------------------------------------------
static PyObject*
alirezanfc_read_card(PyObject* self, PyObject* args)
{
   int sector ;
   bool auth = false , res = false;
   char *sector_data = (char *) malloc(20) ;
   unsigned long card_chip_ID = 0 ;

   if (!PyArg_ParseTuple(args, "i",&sector)) {
        return NULL;
   }
   cout<<"this is sector : "<<sector<<endl;

   memset( sector_data , 0x00 , sizeof( sector_data ) ) ;
   auth = authenticate(sector, 0, false);
   if(auth == true)	{
		card_chip_ID = ((long int) nt.nti.nai.abtUid[0]) + ((long int) nt.nti.nai.abtUid[1]) * 256 	+ ((long int) nt.nti.nai.abtUid[2]) * 65536 + ((long int) nt.nti.nai.abtUid[3]) * 16777215  ;
		if( readblocks(sector ,sector ) )	{
			memcpy(sector_data, mp.mpd.abtData , 16 ) ;		//sector data is read and stored in the "sector_data" array - string size is 16 bytes max and null terminated
			sector_data[15] = 0 ;

			ostringstream card_id_str ;
			card_id_str << card_chip_ID ;

			std::string sector_data_stdstr = "{\"Card_ID\":\"" + card_id_str.str() + "\"},{\"Sector_Data\":\"" + sector_data + "\"}" ;		//json string : "unique card ID":"sector`s data"
			cout<<"this is sector data json: "<<sector_data_stdstr.c_str()<<endl;
			PyObject* card_value = PyUnicode_FromString(const_cast<char*>(sector_data_stdstr.c_str()));
//			free( sector_data ) ;
			return  Py_BuildValue("S", card_value);
		}
	}else	{
//		free( sector_data ) ;
		return Py_None;
	}
}
//-----------------------------------------------------
static PyObject*
alirezanfc_write_card(PyObject* self, PyObject* args)
{
   char *card_value;
   int sector;
   unsigned long card_chip_ID = 0 ;
   if (!PyArg_ParseTuple(args, "si", &card_value, &sector)) {
       return NULL;
   }
   cout<<"this is card value from c++ "<<card_value<<endl;
   cout<<"this is sector : "<<sector<<endl;

   char *tmp_buf = (char *) malloc(20) ;
   memset( tmp_buf , 0x00 , sizeof( tmp_buf ) ) ;

   int	data_lenght = strlen( card_value ) ;
   if( data_lenght > 16 )
	   data_lenght = 16 ;
   memcpy( tmp_buf , card_value , data_lenght ) ;

   if( authenticate(sector, 0, false) )	{
	   card_chip_ID = ((long int) nt.nti.nai.abtUid[0]) + ((long int) nt.nti.nai.abtUid[1]) * 256 	+ ((long int) nt.nti.nai.abtUid[2]) * 65536 + ((long int) nt.nti.nai.abtUid[3]) * 16777215  ;
	   if( writeblock( sector , tmp_buf ) ) {//check if write to card return true otherwise return false

			readblocks(sector ,sector ) ;	//read again to check if write was successfull
			memcpy(tmp_buf , mp.mpd.abtData , 16 ) ;

			ostringstream card_id_str ;
			card_id_str << card_chip_ID ;

			std::string sector_data_stdstr = "{\"Card_ID\":\"" + card_id_str.str() + "\"},{\"Sector_Data\":\"" + tmp_buf + "\"}" ;		//json string : "unique card ID":"sector`s data"
			cout<<"this is sector data json: "<<sector_data_stdstr.c_str()<<endl;
			PyObject* card_value = PyUnicode_FromString(const_cast<char*>(sector_data_stdstr.c_str()));
//			free( tmp_buf ) ;
			return  Py_BuildValue("S", card_value);
	   }
   }else	{
//		free( tmp_buf ) ;
		return Py_None;
   }
}
//-----------------------------------------------------
//-----------------------------------------------------
static PyObject*
alirezanfc_gpio_init(PyObject* self, PyObject* args)
{
   int pin , mode ;

   if (!PyArg_ParseTuple(args, "ii",&pin,&mode)) {
        return NULL;
   }
   cout<<"this is pin number : "<<pin<<endl;
   cout<<"this is pin mode , (0 : in , 1 : out) > "<<mode<<endl;

   if (gpio_init(pin , mode) )
	   return Py_BuildValue("i", 1);
   else
		return Py_BuildValue("i", 0) ;
}
//-----------------------------------------------------
static PyObject*
alirezanfc_gpio_write(PyObject* self, PyObject* args)
{
   int pin , value ;

   if (!PyArg_ParseTuple(args, "ii",&pin,&value)) {
        return NULL;
   }
   cout<<"this is pin number : "<<pin<<endl;
   cout<<"this is pin mode , (0 : in , 1 : out) > "<<value<<endl;

   if( gpio_write(pin , value) )
	   return Py_BuildValue("i", 1) ;
   else
		return Py_BuildValue("i", 0) ;
}
//-----------------------------------------------------

static PyMethodDef Module_methods[] = {
    {"nfc_init", (PyCFunction) alirezanfc_init, METH_NOARGS,
        PyDoc_STR("nfc_init() -> true|false\n\n"
            "initializing nfc c++ lib")},
    {"nfc_polling", (PyCFunction) alirezanfc_polling, METH_NOARGS,
        PyDoc_STR("nfc_polling() -> true|false\n\n"
            "polling nfc c++ lib")},
    {"format_card", (PyCFunction) alirezanfc_format_card, METH_NOARGS,
        PyDoc_STR("format_card() -> true|false\n\n"
            "Format The Card")},
    {"write_to_card", (PyCFunction) alirezanfc_write_card, METH_VARARGS | METH_KEYWORDS,
        PyDoc_STR("write_to_card(card_value,sector) -> card_value\n\n")},
    {"read_from_card", (PyCFunction) alirezanfc_read_card, METH_VARARGS | METH_KEYWORDS,
        PyDoc_STR("read_from_card(sector) -> card_value\n\n")},
    {"gpio_init", (PyCFunction) alirezanfc_gpio_init, METH_VARARGS | METH_KEYWORDS,
        PyDoc_STR("gpio_init(pin, mode) -> true|false\n\n")},
    {"gpio_write", (PyCFunction) alirezanfc_gpio_write, METH_VARARGS | METH_KEYWORDS,
        PyDoc_STR("gpio_write(pin, value) -> true|false\n\n")},

    {NULL, NULL, 0, NULL}  /* Sentinel */
};



//-----------------------------------------------------
#if PY_MAJOR_VERSION >= 3

static int alirezanfc_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int alirezanfc_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}


static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "alirezanfc",
        NULL,
        sizeof(struct module_state),
        Module_methods,
        NULL,
        alirezanfc_traverse,
        alirezanfc_clear,
        NULL
};

#define INITERROR return NULL

PyMODINIT_FUNC
PyInit_alirezanfc(void)

#else
#define INITERROR return

void
initalirezanfc(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
    PyObject *module = PyModule_Create(&moduledef);
#else
    PyObject *module = Py_InitModule3("alirezanfc", Module_methods);
#endif

    if (module == NULL)
        INITERROR;
    struct module_state *st = GETSTATE(module);

    st->error = PyErr_NewException("alirezanfc.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}
