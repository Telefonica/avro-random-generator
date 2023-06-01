package io.confluent.avro.random.generator.geojson;

import io.confluent.avro.random.generator.geojson.crs.CRS;
import io.confluent.avro.random.generator.geojson.crs.EPSG4326;

import java.util.Arrays;
import java.util.List;

/**
 * Use this class in order to create Geometry objects according to GeoJson standard that can be stringified.
 */
public class Geometry {
    public final GeometryType geometryType;
    public final CRS crs;
    public final String coordinates;

    /**
     * Constructor.
     * @param geometryType Geometry type as can be found in GeometryType.java . Random type by default.
     * @param crs          Coordinates Reference System. EPSG4326 by default.
     */
    public Geometry(
            GeometryType geometryType,
            CRS crs
    ) {
        this.geometryType = geometryType == null ? GeometryType.randomGeometryType() : geometryType;
        this.crs = crs == null ? new EPSG4326() : crs;
        coordinates = randomCoordinates();
    }

    /**
     * Stringifies this object.
     * @return String version of this object.
     */
    public String toString() {
        return "{\"type\": \"" + geometryType + "\", \"coordinates\": " + coordinates + "}";
    }

    private String randomCoordinates() {
        switch (geometryType) {
            case Point:
                return new Point(null, null, crs).toString();
            case MultiPoint:
                return Arrays.asList(
                        new Point(null, null, crs).toString(),
                        new Point(null, null, crs).toString()
                ).toString();
            case LineString:
                return Arrays.asList(
                        new Point(null, null, crs).toString(),
                        new Point(null, null, crs).toString(),
                        new Point(null, null, crs).toString()
                ).toString();
            case MultiLineString:
                return Arrays.asList(
                        Arrays.asList(
                                new Point(null, null, crs).toString(),
                                new Point(null, null, crs).toString()
                        ),
                        Arrays.asList(
                                new Point(null, null, crs).toString(),
                                new Point(null, null, crs).toString()
                        )
                ).toString();
            case Polygon:
                String first = new Point(null, null, crs).toString();

                return List.of(
                        Arrays.asList(
                                first,
                                new Point(null, null, crs).toString(),
                                new Point(null, null, crs).toString(),
                                first
                        )
                ).toString();
            case MultiPolygon:
                String first1 = new Point(null, null, crs).toString();
                String first2 = new Point(null, null, crs).toString();

                return Arrays.asList(
                        List.of(
                                Arrays.asList(
                                        first1,
                                        new Point(null, null, crs).toString(),
                                        new Point(null, null, crs).toString(),
                                        first1
                                )
                        ),
                        List.of(
                                Arrays.asList(
                                        first2,
                                        new Point(null, null, crs).toString(),
                                        new Point(null, null, crs).toString(),
                                        first2
                                )
                        )
                ).toString();
            default:
                return null;
        }
    }

    private static class Point {
        private final double latitude;
        private final double longitude;

        public Point(
                Double latitude,
                Double longitude,
                CRS referenceSystemAndProjection
        ) {
            this.latitude = latitude == null ? referenceSystemAndProjection.randomLatitude() : latitude;
            this.longitude = longitude == null ? referenceSystemAndProjection.randomLongitude() : longitude;
        }

        public String toString() {
            return "[" + latitude + ", " + longitude + "]";
        }
    }
}
