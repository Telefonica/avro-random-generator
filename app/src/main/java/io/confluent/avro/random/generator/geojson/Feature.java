package io.confluent.avro.random.generator.geojson;

import java.util.*;

/**
 * Use this class in order to create Feature objects according to GeoJson standard that can be stringified.
 */
public class Feature {
    public final Geometry geometry;
    public final Properties properties;

    /**
     * Constructor.
     *
     * @param geometry Geometry object.
     */
    public Feature(
            Geometry geometry
    ) {
        this.geometry = geometry;
        this.properties = new Properties(geometry.referenceSystemAndProjection);
    }

    /**
     * Stringifies this object.
     *
     * @return String version of this object.
     */
    public String toString() {
        return "{\"type\": \"Feature\", \"geometry\": " + geometry + ", \"properties\": " + properties + "}";
    }

    private static class Properties extends HashMap<String, String> {
        public Properties(ReferenceSystemAndProjection referenceSystemAndProjection) {
            super();
            this.put("\"reference_system\"", "\"" + referenceSystemAndProjection.referenceSystem + "\"");
            this.put("\"projection\"", "\"" + referenceSystemAndProjection.projection + "\"");
            this.put("\"date_time\"", "\"" + new java.util.Date() + "\"");
        }

        @Override
        public String toString() {
            return super.toString().replaceAll("\"=\"", "\": \"");
        }
    }
}
