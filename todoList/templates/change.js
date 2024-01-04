function showEditModal(currentName) {
  // Set the current name in the input field
  document.getElementById('editedName').value = currentName;
  sessionStorage.setItem("currentName", currentName);
  // Show the modal
  $('#editModal').modal('show');
}

function closeEditModal() {
  // Close the modal without saving
  $('#editModal').modal('hide');
}

function saveEditedName() {
  const currentName = sessionStorage.getItem("currentName");
  var editedName = document.getElementById('editedName').value;
    const data = {
    currentName: currentName,
    editedName: editedName
  };

  const headers = new Headers();
  headers.append("Content-Type", "application/json");

  // Make a fetch request with the appropriate data and include credentials
  fetch("/change", {
      method: "POST",
      headers: headers,
      credentials: 'include',
      body: JSON.stringify(data)
  })
  .then(response => {
      if (response.ok) {
          window.location.replace("http://localhost:18080/dashboard");
      } else {
          // Handle errors, for example, if the item couldn't be changed
          alert("Failed to change item");
      }
  })
  .catch(error => {
      console.error("Error:", error);
  });
  // Close the modal after saving
  $('#editModal').modal('hide');
}
