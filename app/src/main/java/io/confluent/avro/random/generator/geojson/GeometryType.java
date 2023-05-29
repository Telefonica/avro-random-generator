package io.confluent.avro.random.generator.geojson;

import java.util.Random;

enum GeometryType {
    Point,
    MultiPoint,
    LineString,
    MultiLineString,
    Polygon,
    MultiPolygon,
    //GeometryCollection
    ;

    public static GeometryType randomGeometryType() {
        Random r = new Random();
        return GeometryType.values()[r.nextInt(GeometryType.values().length)];
    }
}
