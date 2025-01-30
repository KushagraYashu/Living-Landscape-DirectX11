// Texture and sampler for applying textures
Texture2D texture0 : register(t0); // The texture to apply to the geometry
SamplerState Sampler0 : register(s0); // The sampler state for the texture

// Constant buffer for controlling scrolling speed
cbuffer ScrollData : register(b0)
{
    float4 scrollSpeed; // scrollSpeed.x controls the scroll rate, scrollSpeed.y the direction
}

// Input structure containing vertex attributes
struct InputType
{
    float4 position : SV_POSITION; // Vertex position
    float2 tex : TEXCOORD0; // Texture coordinates for mapping the texture
    float3 normal : NORMAL; // Normal vector for lighting (not used here, but passed through)
};

// Main pixel shader function
// Rastertek (2013) DirectX 11 Terrain Tutorial (Lesson 11) (code version 1) [online tutorial]. Adapted from: https://rastertek.com/tertut11.html.
float4 main(InputType input) : SV_TARGET
{
    float4 colour; // Variable to store the texture color
    float2 scrollUV = input.tex; // Get the texture coordinates from the input

    // Apply scrolling effect by modifying the texture's x-coordinate based on the scroll speed
    scrollUV.x += scrollSpeed.y * scrollSpeed.x; // Horizontal scroll based on speed

    // Sample the texture at the modified coordinates
    colour = texture0.Sample(Sampler0, scrollUV);

    // Calculate the luminance of the texture color using a weighted sum of RGB channels
    float luminance = dot(colour.rgb, float3(0.299, 0.587, 0.114));

    // Calculate alpha based on the luminance
    float alpha = pow(luminance, 2.0);

    // Mix the original color with black based on the calculated alpha value
    float4 finalColour = lerp(float4(0, 0, 0, 0), colour, alpha);

    // Apply gamma correction to the final color to convert from linear to sRGB space
    finalColour.rgb = pow(finalColour.rgb, 1 / 2.2);

    return finalColour; // Return the final color to be rendered
}