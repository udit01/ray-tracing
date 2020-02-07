// Compute ray traced image,
/***
 * Steps:
 * 1. Go from eye to pixel(on the canvas) and to which point in the env,
 *  does it connect with.
 * 2. Then connect that point to the light source(s) and if an object 
 * comes it between, then it's a shadow point.
 * 3. Construct for each pixel.
 * 
 * This algorithm is known as backwards tracing or eye tracing as 
 * opposed to forward tracing or light tracing. 
 * 
 */

for (int j = 0; j < imageHeight; ++j) { 
    for (int i = 0; i < imageWidth; ++i) { 
        // compute primary ray direction
        Ray primRay; 
        computePrimRay(i, j, &primRay); 
        // shoot prim ray in the scene and search for intersection
        Point pHit; 
        Normal nHit; 
        float minDist = INFINITY; 
        Object object = NULL; 
        for (int k = 0; k < objects.size(); ++k) { 
            if (Intersect(objects[k], primRay, &pHit, &nHit)) { 
                float distance = Distance(eyePosition, pHit); 
                if (distance < minDistance) { 
                    object = objects[k]; 
                    minDistance = distance; // update min distance 
                } 
            } 
        } 
        if (object != NULL) { 
            // compute illumination
            Ray shadowRay; 
            shadowRay.direction = lightPosition - pHit; 
            bool isShadow = false; 
            for (int k = 0; k < objects.size(); ++k) { 
                if (Intersect(objects[k], shadowRay)) { 
                    isInShadow = true; 
                    break; 
                } 
            } 
        } 
        if (!isInShadow) 
            pixels[i][j] = object->color * light.brightness; 
        else 
            pixels[i][j] = 0; 
    } 
} 


/***
 * Combining reflection, refraction, and etc, by fresnel equation.
 * This algorithm is recursive!
 */ 

// compute reflection color
color reflectionCol = computeReflectionColor(); 
// compute refraction color
color refractionCol = computeRefractionColor(); 
float Kr; // reflection mix value 
float Kt; // refraction mix value 
fresnel(refractiveIndex, normalHit, primaryRayDirection, &Kr, &Kt); 
// mix the two
color glassBallColorAtHit = Kr * reflectionColor + (1-Kr) * refractionColor; 


/***
 * 
 * Pseudo code of tracer
 * 
 */ 

#define MAX_RAY_DEPTH 3 
 
color Trace(const Ray &ray, int depth) 
{ 
    Object *object = NULL; 
    float minDist = INFINITY; 
    Point pHit; 
    Normal nHit; 
    for (int k = 0; k < objects.size(); ++k) { 
        if (Intersect(objects[k], ray, &pHit, &nHit)) { 
            // ray origin = eye position of it's the prim ray
            float distance = Distance(ray.origin, pHit); 
            if (distance < minDistance) { 
                object = objects[i]; 
                minDistance = distance; 
            } 
        } 
    } 
    if (object == NULL) 
        return 0; 
    // if the object material is glass, split the ray into a reflection
    // and a refraction ray.
    if (object->isGlass && depth < MAX_RAY_DEPTH) { 
        // compute reflection
        Ray reflectionRay; 
        reflectionRay = computeReflectionRay(ray.direction, nHit); 
        // recurse
        color reflectionColor = Trace(reflectionRay, depth + 1); 
        Ray refractioRay; 
        refractionRay = computeRefractionRay( 
            object->indexOfRefraction, 
            ray.direction, 
            nHit); 
        // recurse
        color refractionColor = Trace(refractionRay, depth + 1); 
        float Kr, Kt; 
        fresnel( 
            object->indexOfRefraction, 
            nHit, 
            ray.direction, 
            &Kr, 
            &Kt); 
        return reflectionColor * Kr + refractionColor * (1-Kr); 
    } 
    // object is a diffuse opaque object        
    // compute illumination
    Ray shadowRay; 
    shadowRay.direction = lightPosition - pHit; 
    bool isShadow = false; 
    for (int k = 0; k < objects.size(); ++k) { 
        if (Intersect(objects[k], shadowRay)) { 
            // hit point is in shadow so just return
            return 0; 
        } 
    } 
    // point is illuminated
    return object->color * light.brightness; 
} 
 
// for each pixel of the image
for (int j = 0; j < imageHeight; ++j) { 
    for (int i = 0; i < imageWidth; ++i) { 
        // compute primary ray direction
        Ray primRay; 
        computePrimRay(i, j, &primRay); 
        pixels[i][j] = Trace(primRay, 0); 
    } 
} 