#!/usr/bin/env kross

"""
Python script that provides additional formula-functions
to KSpread.

Yahoo! Terms of Use
The feeds are provided free of charge for use by individuals and non-profit
organizations for personal, non-commercial uses. We ask that you provide
attribution to Yahoo! Weather in connection with your use of the feeds.
If you provide this attribution in text, please use: "Yahoo! Weather." If
you provide this attribution with a graphic, please use the Yahoo! Weather
logo that we have included in the feed itself.
 We reserve all rights in and to the Yahoo! Weather logo, and your right
 to use the Yahoo! Weather logo is limited to providing attribution in
 connection with these RSS feeds. 
Yahoo! also reserves the right to require you to cease distributing these
feeds at any time for any reason.

(C)2007 Sebastian Sauer <mail@dipe.org>
http://kross.dipe.org
http://www.koffice.org/kspread
Dual-licensed under LGPL v2+higher and the BSD license.
"""

import urllib
from xml.dom import minidom
import Kross, KSpread

class Yweather:
    def __init__(self, scriptaction):
        self.scriptaction = scriptaction
        #self.currentpath = self.scriptaction.currentPath()

        func = KSpread.function("YWEATHER")
        func.minparam = 1
        func.maxparam = 2
        func.comment = (
            "The YWEATHER() function uses the YAHOO! Weather Service to "
            "display the weather of a location. "
        )
        func.syntax = "YWEATHER(string)"
        func.addParameter("String", "The US zip code, Location ID or cell that contains them.")
        func.addParameter("String", "Units for temperature. f=Fahrenheit and c=Celsius")
        func.addExample("YWEATHER(\"=A1\")")
        func.addExample("YWEATHER(\"GMXX0151\";\"c\")")

        def update(argument):
            print "Yweather.update !!!!!!!!!!!!!!!!!!!!"
            location = argument[0] #"GMXX0151" #hardcoded for now
            if location.startswith('='):
                sheet = KSpread.currentSheet()
                location = sheet.text(location[1:])

            if location == None or location == "":
                func.error = "Invalid location"
                return

            url = "http://weather.yahooapis.com/forecastrss?p=%s" % location
            if len(argument) >= 2:
                url += "&c=%s" % argument[1]

            print "url=%s" % url
            namespace = 'http://xml.weather.yahoo.com/ns/rss/1.0'
            dom = None
            try:
                dom = minidom.parse(urllib.urlopen(url))
            except:
                func.error = "Web services request failed"
                return

            forecasts = []
            for node in dom.getElementsByTagNameNS(namespace, 'forecast'):
                forecasts.append({
                    'date': node.getAttribute('date'),
                    'low': node.getAttribute('low'),
                    'high': node.getAttribute('high'),
                    'condition': node.getAttribute('text')
                })

            try:
                ycondition = dom.getElementsByTagNameNS(namespace, 'condition')[0]
            except IndexError:
                func.error = "Invalid condition"
                return
            #my_current_condition = ycondition.getAttribute('text')
            #my_current_temp = ycondition.getAttribute('temp')
            #my_forecasts = forecasts
            #my_title = dom.getElementsByTagName('title')[0].firstChild.data

            temp = ycondition.getAttribute('temp')
            print "Y! Weather Temperatur: %s" % temp
            func.result = temp

        func.connect("called(QVariantList)", update)
        func.registerFunction()

print "Started Y! Weather"
Yweather( self )
print "Finished Y! Weather"
