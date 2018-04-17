
# https://pypi.org/project/Shapely/
# https://shapely.readthedocs.io/en/latest/



import pdb

import sys
import numpy as np
import math
from sets import Set

# if having trouble installing: https://stackoverflow.com/a/49335954/23630
import matplotlib.pyplot as plt
fig, ax = plt.subplots()

from shapely.ops import polygonize
from shapely.geometry import Polygon
from shapely.geometry import MultiPolygon
from shapely.geometry import MultiLineString
from shapely.geometry import CAP_STYLE, JOIN_STYLE
from shapely.ops import cascaded_union
from shapely.geometry import box
from shapely.geometry import LineString
from shapely.geometry import LinearRing
from shapely.geometry import Point

def plot_coords(coords):
    pts = list(coords)
    x,y = zip(*pts)
    plt.plot(x,y)

def plot_polys(polys):
    for poly in polys:
        if (not getattr(poly, "exterior", None)):
            print("got line?")

        plot_coords(poly.exterior.coords)

        for hole in poly.interiors:
            plot_coords(hole.coords)

if 0:
    boxes = MultiPolygon([box(0, 0, 10,100),
                          box(20,20,30,100)])
    plot_polys(boxes)
    plt.show()

    box1 = box(0, 0, 10,100)
    boxi1 = box1.buffer(7, join_style=JOIN_STYLE.mitre)
    box2 = box(20,20,30,100)
    boxi2 = box2.buffer(7, join_style=JOIN_STYLE.mitre)

    plot_polys([box1, boxi1, box2, boxi2])
    plt.show()

    inter = boxi1.intersection(boxi2)

    plot_polys([box1, boxi1, box2, boxi2, inter])
    plt.show()





    p1 = Point(10,20)
    plot_polys([p1.buffer(5)])
    plt.show()

    l1 = LineString([(10,20), (30,40)])
    plot_coords(l1.coords)
    plt.show()

    mls1 = MultiLineString([((0,0),(1,1)),((2,2),(3,3))])
    for g in mls1.geoms:
        plot_coords(g.coords)
    plt.show()

    mls2 = MultiLineString([LineString([(0,0),(0,1)]),((0,1),(1,1),(1,0),(0,0))])
    p2 = polygonize(mls2)
    plot_polys(p2)
    plt.show()

    mls3 = MultiLineString([((0,0),(0,10),(10,10),(10,0),(0,0)),
                           ((5,5),(5,7), (7,7),  (7,5), (5,5))
    ])
    # gives two polygons
    print("showing polygonized")
    for p in polygonize(mls3):
        plot_polys([p])
        plt.show()
    print("done")

    sys.exit()

# poly coords
# poly holes
# use list to extract




# four polygons. a big one surrounding them all. two intermediate ones, each with one inside them
mls4 = MultiLineString([((0,0),(0,10),(10,10),(10,0),(0,0)),
                        ((5,5),(5,7), (7,7),  (7,5), (5,5)),
                        ((20,0),(20,10),(30,10), (30,0), (20,0)),
                        ((25,5),(25,7), (27,7),  (27,5), (25,5)),
                        ((-50,-50),(-50,50),(50,50),(50,-50),(-50, -50))
])
if 0:

    for g in mls4.geoms:
        plot_coords(g.coords)
    plt.show()

    for p in polygonize(mls4):
        plot_polys([p])
        plt.show()


class Contains(object):
    def __init__(self, o):
        self.o = o
    def __lt__(self, other):
        return self.o.contains(other.o)

if 0:
    print("now showing sorted")
    for p in sorted(polygonize(mls4), key=Contains):
        plot_polys([p])
        plt.show()

polys = list(polygonize(mls4))
print("0 contains 1? {}".format(polys[0].contains(polys[1])))
print("1 contains 0? {}".format(polys[1].contains(polys[0])))

print("0 exterior contains 1 exterior? {}".format(polys[0].exterior.contains(polys[1].exterior)))

print("0 exterior polygon contains 1 exterior? {}".format(Polygon(polys[0].exterior).contains(polys[1].exterior)))



class ContainsExteriorPolygon(object):
    def __init__(self, o):
        self.o = o
    def __gt__(self, other):
        return Polygon(self.o.exterior).contains(other.o)

if 0:
    for p in sorted(polygonize(mls4), key=ContainsExteriorPolygon, reverse=True):
        plot_polys([p])
        plt.show()




union1 = cascaded_union(list(polygonize(mls4)))

if 1:

    for g in mls4.geoms:
        plot_coords(g.coords)
    plt.show()


    plot_polys([union1])
    plt.show()




# why do I need a list() around polygonize?
union2 = cascaded_union(list(polygonize(MultiLineString([((0,0),(0,10),(10,10),(10,0),(0,0)),
                                                         ((20,0),(20,10),(30,10), (30,0), (20,0))]))))

# this works. note union2 - two objects.
plot_polys(MultiPolygon(union2))
plt.show()

# this doesn't work
plot_polys(MultiPolygon(union1))




# this does...
plot_polys(MultiPolygon([union1]))
plt.show()

# this doesn't crash, but it doesn't give the expected results.
plot_polys(MultiPolygon([union2]))
plt.show()

# from the shapely manual
# class MultiPolygon(polygons)
# The MultiPolygon constructor takes a sequence of exterior ring and hole list tuples: [((a1, ..., aM), [(b1, ..., bN), ...]), ...].

if isinstance(union1, Polygon):
    plot_polys([union1])
else:
    plot_polys(union1)
plt.show()

if isinstance(union2, Polygon):
    plot_polys([union2])
else:
    plot_polys(union2)
plt.show()


# two intersecting polygons. the result is not a union.
mls5 = MultiLineString([((0,0),(0,10),(10,10),(10,0),(0,0)),
                       ((5,5),(5,15),(15,15),(15,5),(5,5))])


if 1:
    for g in mls5.geoms:
        plot_coords(g.coords)
    plt.show()

    for p in polygonize(mls5):
        plot_polys([p])
        plt.show()
