**NFC Python C Extension library using PN532 to read/write card :**



## Building

First you need a C compiler and the Python development libraries. Then:

```
python3 setup.py build
```

## Using


```
Python 3.5.2 (default, Nov 12 2018, 13:43:14) 
[GCC 5.4.0 20160609] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import alirezanfc as nfc
>>> help(nfc)
Help on module alirezanfc:

NAME
    alirezanfc

FUNCTIONS
    format_card(...)
        format_card() -> true|false
        
        Format The Card
    
    gpio_init(...)
        gpio_init(pin, mode) -> true|false
    
    gpio_write(...)
        gpio_write(pin, value) -> true|false
    
    nfc_init(...)
        nfc_init() -> true|false
        
        initializing nfc c++ lib
    
    nfc_polling(...)
        nfc_polling() -> true|false
        
        polling nfc c++ lib
    
    read_from_card(...)
        read_from_card(sector) -> card_value
    
    write_to_card(...)
        write_to_card(card_value,sector) -> card_value

```










