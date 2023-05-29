package io.confluent.avro.random.generator.geojson;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Use this class in order to create Geometry objects according to GeoJson standard that can be stringified.
 */
public class Geometry {
    private static final ReferenceSystem DEF_REF_SYSTEM = ReferenceSystem.ETRS89;
    private static final Projection DEF_PROJECTION = Projection.UTM;
    public final GeometryType geometryType;
    public final ReferenceSystemAndProjection referenceSystemAndProjection;
    public final String coordinates;

    /**
     * Constructor.
     * @param geometryType                 Geometry type as can be found in GeometryType.java . Random type by default.
     * @param referenceSystemAndProjection Reference system and projection as can be found in
     *                                     ReferenceSystemAndProjection.java . DEF_REF_SYSTEM by default.
     */
    public Geometry(
            GeometryType geometryType,
            ReferenceSystemAndProjection referenceSystemAndProjection
    ) {
        this.geometryType = geometryType == null ? GeometryType.randomGeometryType() : geometryType;
        this.referenceSystemAndProjection = referenceSystemAndProjection == null ?
                new ReferenceSystemAndProjection(DEF_REF_SYSTEM, DEF_PROJECTION) : referenceSystemAndProjection;
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
                return new Point(null, null, referenceSystemAndProjection).toString();
            case MultiPoint:
                return Arrays.asList(
                        new Point(null, null, referenceSystemAndProjection).toString(),
                        new Point(null, null, referenceSystemAndProjection).toString()
                ).toString();
            case LineString:
                return Arrays.asList(
                        new Point(null, null, referenceSystemAndProjection).toString(),
                        new Point(null, null, referenceSystemAndProjection).toString(),
                        new Point(null, null, referenceSystemAndProjection).toString()
                ).toString();
            case MultiLineString:
                return Arrays.asList(
                        Arrays.asList(
                                new Point(null, null, referenceSystemAndProjection).toString(),
                                new Point(null, null, referenceSystemAndProjection).toString()
                        ),
                        Arrays.asList(
                                new Point(null, null, referenceSystemAndProjection).toString(),
                                new Point(null, null, referenceSystemAndProjection).toString()
                        )
                ).toString();
            case Polygon:
                String first = new Point(null, null, referenceSystemAndProjection).toString();

                return List.of(
                        Arrays.asList(
                                first,
                                new Point(null, null, referenceSystemAndProjection).toString(),
                                new Point(null, null, referenceSystemAndProjection).toString(),
                                first
                        )
                ).toString();
            case MultiPolygon:
                String first1 = new Point(null, null, referenceSystemAndProjection).toString();
                String first2 = new Point(null, null, referenceSystemAndProjection).toString();

                return Arrays.asList(
                        List.of(
                                Arrays.asList(
                                        first1,
                                        new Point(null, null, referenceSystemAndProjection).toString(),
                                        new Point(null, null, referenceSystemAndProjection).toString(),
                                        first1
                                )
                        ),
                        List.of(
                                Arrays.asList(
                                        first2,
                                        new Point(null, null, referenceSystemAndProjection).toString(),
                                        new Point(null, null, referenceSystemAndProjection).toString(),
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
                ReferenceSystemAndProjection referenceSystemAndProjection
        ) {
            this.latitude = latitude == null ? referenceSystemAndProjection.randomLatitude() : latitude;
            this.longitude = longitude == null ? referenceSystemAndProjection.randomLongitude() : longitude;
        }

        public String toString() {
            return "[" + latitude + ", " + longitude + "]";
        }
    }
}
