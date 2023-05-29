package io.confluent.avro.random.generator.geojson;

import java.util.ArrayList;

/**
 * Use this class in order to create FeatureCollection objects according to GeoJson standard that can be
 * stringified.
 */
public class FeatureCollection {
    public ArrayList<Feature> features;

    /**
     * Constructor.
     * @param features Features list.
     */
    public FeatureCollection(
            ArrayList<Feature> features
    ) {
        this.features = features;
    }

    /**
     * Stringifies this object.
     * @return String version of this object.
     */
    public String toString() {
        return "{\"type\": \"FeatureCollection\", \"features\":" + features + "}";
    }
}
