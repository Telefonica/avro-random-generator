package io.confluent.avro.random.generator.geojson;

import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.google.gson.JsonSyntaxException;
import io.confluent.avro.random.generator.geojson.crs.CRS;
import io.confluent.avro.random.generator.geojson.crs.EPSG4326;
import org.junit.Test;

import static junit.framework.TestCase.*;

public class GeometryTest {
    GeometryType geometryType = GeometryType.Point;
    CRS crs = new EPSG4326();

    @Test
    public void shouldCreateGeometryWithDefaults() {
        Geometry geometry = new Geometry(null, null);
        assertNotNull(geometry);
        assertNotNull(geometry.geometryType);
        assertNotNull(geometry.coordinates);
        System.out.println(geometry);
    }

    @Test
    public void shouldCreateGeometryWithSpecificGeometryType() {
        Geometry geometry = new Geometry(geometryType, null);
        assertNotNull(geometry);
        assertEquals(geometry.geometryType, geometryType);
        assertNotNull(geometry.coordinates);
        System.out.println(geometry);
    }

    @Test
    public void shouldCreateGeometryWithSpecificReferenceSystem() {
        Geometry geometry = new Geometry(null, crs);
        assertNotNull(geometry);
        assertNotNull(geometry.geometryType);
        assertNotNull(geometry.coordinates);
        assertEquals(geometry.crs, crs);
        System.out.println(geometry);
    }

    @Test
    public void shouldStringifyCorrectly() {
        Geometry geometry = new Geometry(geometryType, crs);
        assertNotNull(geometry.toString());
        Gson gson = new Gson();

        try {
            gson.fromJson(geometry.toString(), JsonObject.class);
        } catch (JsonSyntaxException exception) {
            fail();
        }

        System.out.println(geometry);
    }
}
