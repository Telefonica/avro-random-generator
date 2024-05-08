package io.confluent.avro.random.generator.geojson;

import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.google.gson.JsonSyntaxException;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Random;

import static junit.framework.TestCase.*;

public class FeatureCollectionTest {
    @Test
    public void shouldCreateFeatureCollection() {
        FeatureCollection featureCollection = createFeatureCollection();
        assertNotNull(featureCollection);
        System.out.println(featureCollection);
    }

    @Test
    public void shouldStringifyCorrectly() {
        FeatureCollection featureCollection = createFeatureCollection();
        assertNotNull(featureCollection.toString());
        Gson gson = new Gson();
        String fcollection = featureCollection.toString();
        try {
            gson.fromJson(fcollection, JsonObject.class);
        } catch (Exception exception) {

            exception.printStackTrace();
            fail();
        }

        System.out.println(featureCollection);
    }

    private FeatureCollection createFeatureCollection() {
        Random r = new Random();
        int maxFeatures = 10;
        int numFeatures = r.nextInt(maxFeatures) + 1;
        Feature[] features = new Feature[numFeatures];

        for (int i = 0; i < numFeatures; i++) {
            features[i] = new Feature(new Geometry(null, null));
        }

        return new FeatureCollection(new ArrayList<>(Arrays.asList(features)));
    }
}
