#! /usr/bin/python
# Evaluate the result of the classifier
#
# Copyright (C) 2008  Diego Molla-Aliod <diego@ics.mq.edu.au>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
##############################################################################

"Evaluate the result of the classifier"

import sys

if len(sys.argv) == 3:
    gold_st = sys.argv[1]
    prediction = sys.argv[2]
else:
    gold_st = raw_input("Filename of the gold standard: ")
    prediction = raw_input("Filename of the predicted classes: ")

gold_f = open(gold_st,"r")
pred_f = open(prediction,"r")

gold_f.readline()

correct = dict()
total = dict()
predicted = dict()

gold_s = gold_f.readline()
pred_s = pred_f.readline()
#count = 0
while gold_s != "": # and count < 1000:
    target = gold_s.split()[0]
    predict = pred_s.split()[0]
    total[target] = total.get(target,0) + 1
    predicted[predict] = predicted.get(predict,0) + 1
    if target == predict:
        correct[target] = correct.get(target,0) + 1
    gold_s = gold_f.readline()
    pred_s = pred_f.readline()
    #count = count+1

correct_sum = 0.0
total_sum = 0.0
predicted_sum = 0.0
for c in total.keys():
    correct_n = correct.get(c,0)+0.0
    total_n = total.get(c,0)+0.0
    predicted_n = predicted.get(c,0)+0.0
    
    if total_n == 0:
        recall = 0.0
    else:
        recall = correct_n*100/total_n

    if predicted_n == 0:
        precision = 0.0
    else:
        precision = correct_n*100/predicted_n
        
    print "Class %s R = %.2f%% P = %.2f%%" % (c, recall, precision)

    if c == "0":
      continue
    correct_sum += correct_n
    total_sum += total_n
    predicted_sum += predicted_n

if total_sum > 0:
   recall = correct_sum*100/total_sum
   precision = correct_sum*100/predicted_sum

   print "micro-averaged excluding class 0: R = %.2f%% P = %.2f%%" % (recall,precision)
