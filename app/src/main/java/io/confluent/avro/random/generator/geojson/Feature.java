package io.confluent.avro.random.generator.geojson;

import io.confluent.avro.random.generator.geojson.crs.CRS;

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
        this.properties = new Properties(geometry.crs);
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
        public Properties(CRS crs) {
            super();
            this.put("\"crs\"", "\"" + crs.toString() + "\"");
            this.put("\"date_time\"", "\"" + new java.util.Date() + "\"");
        }

        @Override
        public String toString() {
            return super.toString().replaceAll("\"=\"", "\": \"");
        }
    }
}
