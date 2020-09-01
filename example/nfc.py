from apscheduler.schedulers.background import BackgroundScheduler
import time
from sqlite_orm.database import Database
from sqlite_orm.field import IntegerField, BooleanField, TextField
from sqlite_orm.table import BaseTable
import requests
import os
import alirezanfc as nfc
import json

nfc.nfc_init()

REFRESH_INTERVAL = 30  # seconds
REFRESH_INTERVAL_CARD = 1  # seconds

scheduler = BackgroundScheduler()
scheduler.start()


class Card(BaseTable):
    __table_name__ = 'card'

    id = IntegerField(primary_key=True, auto_increment=True)
    card_id = TextField(not_null=True)


def dict_factory(cursor, row):
    d = {}
    for idx, col in enumerate(cursor.description):
        d[col[0]] = row[idx]
    return d


if not os.path.isfile("nfc.db"):
    db = Database("nfc.db")
    db.query(Card).create().execute()


def job():
    db1 = Database("nfc.db")
    chip_id = "0281d7950e5b"
    server = "http://yourdomain.ir/api/cards/" + chip_id
    r = None
    try:
        r = requests.get(server)
    except:
        pass

    if r is not None:
        data = r.json()['data']
        ids = []
        cursor = db1.query(Card).select().execute()
        for row in cursor:
            row = dict_factory(cursor, row)
            del row['id']
            ids.append(row)
        ids = list(ids)
        print("this is ids : ", ids)
        print("this is data : ", data)
        for item in ids:
            if item in data:
                print("has")
            else:
                print("not has")
                db1.query(Card).delete().filter(Card.card_id == item['card_id']).execute()
        for item in data:
            # check card not exists
            cursor = db1.query(Card).select().filter(Card.card_id == item['card_id']).execute()
            if len(cursor.fetchall()) == 0:
                card = Card(card_id=item['card_id'])
                db1.query(Card).insert(card).execute()

        cursor = db1.query(Card).select().execute()
        print("this is fetchall : ", cursor.fetchall())


def card_reader_job():
    db2 = Database("nfc.db")
    cursor = None
    value = None
    if nfc.nfc_polling():
        value = nfc.read_from_card(2)
        try:
            value = value.split(',')
            value = json.loads(value[0])
            print(value)
            cursor = db2.query(Card).select().filter(Card.card_id == value['Card_ID']).execute()
            if len(cursor.fetchall()) == 0:
                print("has not permission")
            else:
                if nfc.gpio_init(6, 1):
                    nfc.gpio_write(6, 1)
                    time.sleep(.1)
                    nfc.gpio_write(6, 0)
                print("has permission")
        except:
            import traceback
            print(traceback.print_exc())
            print("error")
    del db2
    del value
    del cursor


# then every 60 seconds after that.
scheduler.add_job(job, 'interval', seconds=REFRESH_INTERVAL)
scheduler.add_job(card_reader_job, 'interval', seconds=REFRESH_INTERVAL_CARD)

# cursor = db.query(Card).select().filter(Card.card_id == '478971958').execute()
# print(cursor.fetchall())

# main loop
while True:
    time.sleep(2)
