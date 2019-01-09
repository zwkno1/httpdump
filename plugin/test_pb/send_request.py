#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import requests
import test_pb2

if len(sys.argv) != 2 :
    print("usage: " + sys.argv[0] + " " + "url")
    sys.exit(1)

company = test_pb2.Company()
company.name = 'haha'
for i in range(10):
    p = company.employees.add()
    p.id = i
    p.name = 'employee ' + str(i)
    p.age = i*20
    p.phone = '100000'+ str(i)
    p.salary = i*388.23

data = company.SerializeToString()

url = sys.argv[1]
r = requests.post(url, data=data)
print(r.text)



