package io.confluent.avro.random.generator.geojson;

import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.google.gson.JsonSyntaxException;
import org.junit.Test;

import static junit.framework.TestCase.*;

public class FeatureTest {
    GeometryType geometryType = GeometryType.Point;
    ReferenceSystemAndProjection referenceSystemAndProjection = new ReferenceSystemAndProjection(
            ReferenceSystem.ETRS89,
            Projection.UTM
    );
    Geometry geometry = new Geometry(geometryType, referenceSystemAndProjection);

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
