package io.confluent.avro.random.generator.geojson;

import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.google.gson.JsonSyntaxException;
import io.confluent.avro.random.generator.geojson.crs.CRS;
import io.confluent.avro.random.generator.geojson.crs.EPSG4326;
import org.junit.Test;

import static junit.framework.TestCase.*;

public class FeatureTest {
    GeometryType geometryType = GeometryType.Point;
    CRS crs = new EPSG4326();
    Geometry geometry = new Geometry(geometryType, crs);

    @Test
    public void shouldCreateFeature() {
        Feature feature = new Feature(geometry);
        assertEquals(feature.geometry, geometry);
        assertNotNull(feature);
        assertNotNull(feature.properties);
        System.out.println(feature);
    }

    @Test
    public void shouldStringifyCorrectly() {
        Feature feature = new Feature(geometry);
        assertNotNull(feature.toString());
        Gson gson = new Gson();

        try {
            gson.fromJson(feature.toString(), JsonObject.class);
        } catch (JsonSyntaxException exception) {
            fail();
        }

        System.out.println(feature);
    }
}
