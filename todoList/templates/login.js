document.getElementById("loginButton").addEventListener("click", submitForm);

function submitForm() {
    const username = document.getElementById("username").value;
    const password = document.getElementById("password").value;

    // Combine username and password and encode in base64
    const credentials = btoa(`${username}:${password}`);

    // Prepare headers for the POST request
    const headers = new Headers();
    headers.append("Authorization", `Basic ${credentials}`);
    headers.append("Content-Type", "application/x-www-form-urlencoded");

    // Make a fetch request with the Authorization header
    fetch("/login", {
        method: "POST",
        headers: headers,
        credentials: 'include'
    })
    .then(response => {
        if (response.ok) {
            window.location.replace("https://todoapp-a7a9f5a0e861.herokuapp.com/dashboard");
        } else {
            // Handle authentication failure or other errors
            alert("Wrong Password");
        }
    })
    .catch(error => {
        console.error("Error:", error);
    });
}
